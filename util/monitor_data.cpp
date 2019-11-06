// CopyRight 2019 360. All rights reserved.
// File   monitor_data.cpp
// Date   2019-10-28 16:48:25
// Brief

#include <iostream>
#include <sstream>
#include <algorithm>
#include "util/monitor_data.h"
#include "util/time_utils.h"

namespace prediction {
namespace util {
namespace monitor {

// static const int kStatLen = 10 ; // time len of data to save
// static const int kMetricLen = 1500 ;// max mertrics num to montior
// static const int kNameLen = 100;

uint64_t hash_str(const std::string& str) {
  size_t n = str.size();
  if (n == 0) {
    return 0;
  }
  const char* p = str.c_str();
  uint64_t h = 2654435761u * (*p);
  while (--n) {
    h = (h * 97) + *++p;
  }
  return h;
}

MetricItem::MetricItem() {
  Reset();
}

int MetricItem::AddItem(const char* name, const MonitorItem& item) {
  if (!IsNameMatch(name)) {
    Reset();
    SetName(name);
  }
  int idx = GetDataIndex(item.timestamp) ;
  return _monitor_items[idx].AddValue(item); 
}

int MetricItem::SetName(const char* name) {
  if (name == NULL) {
    return -1;
  }
  strncpy(_name, name, kNameLen - 1);
  _name[kNameLen - 1] = '\0';
  _name_hash_key = hash_str(name); 
  return 0;
}

int MetricItem::GetDataIndex(long timestamp) {
  int idx = 0;
  bool found = false;
  for (int i = 0; i < kStatLen; i++) {
    if (_monitor_items[i].timestamp == timestamp) {
      found = true ;
      idx = i; // old index
      break;
    }
  }
  if (!found) {
    idx = _cursor;
    _monitor_items[idx].Reset();// reset, prepare to new data writting 
    _cursor = (_cursor + 1) % kStatLen;
  }
  return idx;
}

bool MetricItem::IsValid() const {
  return (_name_hash_key > 0 || strlen(_name) != 0);
}

bool MetricItem::IsNameMatch(const char* name, uint64_t hash_val) const {
  bool ret = false;
  if (hash_val > 0) {
    ret = (_name_hash_key == hash_val);
  } else if (name != NULL) {
    ret = (strcmp(_name, name) == 0);
  }
  return ret;
}

void MetricItem::Reset() {
  memset(_name, 0, kNameLen);
  _cursor = 0;
  _name_hash_key = 0;
  for (int i = 0; i < kStatLen; ++i) {
    _monitor_items[i].Reset(); // reset item
  }
}

MonitorData::MonitorData() : _init_flag(0), _cursor(0) {
  Reset();
}

void MonitorData::Init() {
  if (_init_flag != 1) {
    _init_flag = 1;
    Reset();
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    // process mutex
    // TODO(lidongming):check macro _POSIX_SHARED_MEMORY_OBJECTS
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ADAPTIVE_NP);
    pthread_mutex_init(&_mutex, &attr);
  }
} 

void MonitorData::Reset() {
  _cursor = 0;
  for (int i = 0; i < kMetricLen; i++) {
    _metric_items[i].Reset();
  }
}

void MonitorData::Lock() { 
  pthread_mutex_lock(&_mutex);
}

void MonitorData::UnLock() {
  pthread_mutex_unlock( &_mutex );
}

int MonitorData::GetDataIndex(const char* name) {
  int index = 0;
  bool new_metric = true;
  uint64_t hash_name_key = hash_str(name);
  for (int i = 0; i < kMetricLen; ++i) {
    if (_metric_items[i].IsNameMatch(name, hash_name_key)) {
      new_metric = false;
      index = i;
      break;
    }
  }

  if (new_metric) {
    index = _cursor;
    _metric_items[index].Reset();
    _cursor = (_cursor + 1) % kMetricLen;
  }

  return index;
}

int MonitorData::Update(const char* key, const MonitorItem& item) {
  int ret = 0;
  if (_init_flag != 1) {
    Init();
  }

  if (key == NULL) { 
    // LOG_ERROR("empty key_val %s ",key);
    return -1;
  }

  char name[kNameLen] = { '\0' };
  strncpy(name, key, kNameLen - 1);
  name[kNameLen - 1] = '\0';

  int index = GetDataIndex(name);
  ret = _metric_items[index].AddItem(key, item);

  return ret;
}

std::string GetFormatTimeStr(long timestamp) {
  std::stringstream ss;
  time_t curtime = timestamp;
  tm* ltm = localtime(&curtime);

  if (ltm != NULL ){
    ss << 1900 + ltm->tm_year;

    if (ltm->tm_mon < 9) ss << "0";
    ss << ltm->tm_mon + 1;

    if (ltm->tm_mday < 10) ss << "0";
    ss << ltm->tm_mday << "_";


    if (ltm->tm_hour < 10) ss << "0";
    ss << ltm->tm_hour << ":";

    if (ltm->tm_min < 10) ss << "0";
    ss << ltm->tm_min << ":";

    if (ltm->tm_sec < 10) ss << "0";
    ss << ltm->tm_sec;
  } else {
    ss << "[ERROR_TIME]";
  }
  return ss.str();
}

void MonitorData::Dump(std::string* dump_str) {
  if (_init_flag != 1) { return; }

  // copy
  Lock();
  MonitorData _data(*this);
  UnLock();

  std::string item_name;
  std::stringstream ss;
  ss.precision(3);
  ss << "## date  \t time(sec) \t key \t val \t type"<<std::endl;
  std::string key;
  std::string time_;
  std::string val;

  for(int i = 0; i < kMetricLen; i++) {
    MetricItem& item = _data._metric_items[i];
    if (item.IsValid()) {
      key = item._name;
      std::sort(item._monitor_items, item._monitor_items + kStatLen,
          MonitorItem::compare);

      for (int j = 0; j < kStatLen; j++) {
        const MonitorItem& data = item._monitor_items[j];
        if (data.IsValid()) {
          if (data.operation & AVG) {
            ss << std::fixed << GetFormatTimeStr(data.timestamp) << "\t"
              << data.timestamp << "\t" << key << "\t"
              << (double)data.sum / data.count << "\t"
              << "AVG" << std::endl;
          }
          if (data.operation & INC) {
            ss << std::fixed << GetFormatTimeStr(data.timestamp) << "\t"
              << data.timestamp << "\t" << key << "\t"
              << (double)data.sum << "\t"
              << "INC" << std::endl; 
          } 
          if (data.operation & MIN) {
            ss << std::fixed << GetFormatTimeStr(data.timestamp) << "\t"
              << data.timestamp << "\t" << key << "\t"
              << (double)data.min << "\t"
              << "MIN" << std::endl;
          }
          if (data.operation & MAX) {
            ss << std::fixed << GetFormatTimeStr(data.timestamp) << "\t"
              << data.timestamp << "\t" << key << "\t"
              << (double)data.max << "\t"
              << "MAX" << std::endl;
          }
        }
      }
    }
  }
  *dump_str = ss.str(); 
}

void MonitorData::RawDump(std::string* dump_str) {
  if (_init_flag != 1) {
    *dump_str = "";
  }

  // copy
  Lock();
  MonitorData _data(*this);
  UnLock();

  std::stringstream ss;
  ss.precision(3);
  ss << "## date  \t name \t sum \t count \t "
    << "time(sec) \t max \t min\t op"
    << std::endl;

  std::string item_name;
  std::string key;
  std::string time_;
  std::string val;

  for(int i = 0; i < kMetricLen; ++i){
    MetricItem& item = _data._metric_items[i];
    if (item.IsValid()) {
      key = item._name;
      std::sort(item._monitor_items, item._monitor_items + kStatLen,
          MonitorItem::compare);

      for (int j = 0; j < kStatLen; j++){
        const MonitorItem& data = item._monitor_items[j];
        if (data.IsValid()) {
          ss << std::fixed << GetFormatTimeStr(data.timestamp) << "\t"
            << key << "\t" << data.sum << "\t" << data.count << "\t"
            << data.timestamp << "\t" << data.max << "\t"
            << data.min << "\t" << data.operation << std::endl;
        }
      }
    }
  }
  *dump_str = ss.str(); 
}

}  // namespace monitor
}  // namespace util
}  // namespace prediction

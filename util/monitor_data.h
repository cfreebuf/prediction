// CopyRight 2019 360. All rights reserved.
// File   monitor_data.h
// Date   2019-10-28 16:47:50
// Brief

#ifndef PREDICTION_SERVER_UTIL_MONITOR_DATA_H_
#define PREDICTION_SERVER_UTIL_MONITOR_DATA_H_

#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <error.h>
#include <string>
#include <limits> 

namespace prediction {
namespace util {
namespace monitor {

// extern int kStatLen; // time len of data to save
// extern int kMetricLen;// max mertrics num to montior
// extern int kNameLen;

#define kStatLen 10 // time len of data to save
#define kMetricLen 1500 // max mertrics num to montior
#define kNameLen 100

#define MAX_VAL  0.0
#define MIN_VAL  std::numeric_limits<double>::max()

typedef enum  {
    UNDEFINED = 0,
    AVG = 1,
    INC = 1<<1,
    MIN = 1<<2,
    MAX = 1<<3,
    ALL = 15
} Operation;

// Basic monitor item
struct MonitorItem {
 public:
  double sum;
  long   count;
  long   timestamp; // time of record 
  double max;
  double min;
  int operation;   

 public:   
  MonitorItem() { this->Reset(); }     

  void UpdateMaxMin(double max, double min){
      if (max > this->max) this->max = max; 
      if (min < this->min) this->min = min;
  }

  int AddValue(int operation, double value, long timestamp) {
      if (timestamp != this->timestamp) {
          Reset();
          this->timestamp = timestamp;
      }
      sum += value;
      count++;
      UpdateMaxMin(value, value);
      this->operation = this->operation | operation;
      return 0;
  } 

  int AddValue(const MonitorItem& item) {
      if (item.timestamp != this->timestamp) {
          this->Reset();
          this->timestamp = item.timestamp;
      }        
      this->sum += item.sum;
      this->count += item.count;
      this->UpdateMaxMin(item.max, item.min);
      this->operation = this->operation | item.operation;
      return 0;
  }

  bool IsValid() const {
      return this->timestamp > 0 && this->count > 0;
  }

  void Reset() {
      this->sum = 0;
      this->count = 0 ;
      this->timestamp = 0;
      this->max = MAX_VAL;
      this->min = MIN_VAL;
      this->operation = UNDEFINED;
  }

  static bool compare(const MonitorItem& a, const MonitorItem& b) {
      return a.timestamp > b.timestamp;
  }
};  // MonitorItem

class MetricItem {
 public:
  char _name[kNameLen];
  uint64_t _name_hash_key;
  MonitorItem _monitor_items[kStatLen];
  int _cursor; // overwrite cursor  

 public:
  MetricItem();

  int AddItem(const char* name, const MonitorItem& item);
  int SetName(const char* name); 
  int GetDataIndex(long timestamp);
  bool IsValid() const;
  bool IsNameMatch(const char* name, uint64_t hash_val = 0) const;
  void Reset();
};

struct MonitorData {
 private:
  int _init_flag;
  int _cursor;
  MetricItem _metric_items[kMetricLen];
  pthread_mutex_t _mutex;

  void Reset();

 public:
  MonitorData();
  void Init();
  void Lock();
  void UnLock(); 

  int GetDataIndex(const char* name);
  int Update(const char* key, const MonitorItem& item);

  void RawDump(std::string* dump_str);
  void Dump(std::string* dump_str);
};  // MetricItem

} // namespace monitor
} // namespace util
} // namespace prediction

#endif  // PREDICTION_SERVER_UTIL_MONITOR_DATA_H_

// CopyRight 2019 360. All rights reserved.
// File   reloader_entry.h
// Date   2019-10-29 11:01:57
// Brief

#ifndef RELOADER_RELOADER_ENTRY_H_
#define RELOADER_RELOADER_ENTRY_H_

#include <sys/stat.h>
#include <string.h>
#include <new>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <malloc.h>
#include <memory>
#include "util/logging.h"
#include "util/redis_client.h"

namespace prediction {
namespace util {
namespace reloader {

using util::RedisClient;

#define kOldBufferWaitSeconds 10
// TODO(lidongming): use c++11 memory order instead
#define RELOADER_MEMORY_BARRIER() asm volatile("" ::: "memory")
const size_t kMemSize = 1024llu * 1024 * 1024 * 3;

enum ReloaderState {
  STATE_NOT_CHANGED = 0,
  STATE_NOT_TIME,
  STATE_CHANGED,
  STATE_UNKNOWN
};

enum StateCheckerType {
  STATE_CHECKER_FILE = 0,
  STATE_CHECKER_REDIS,
  STATE_CHECKER_UNKNOWN = 100,
};

struct ReloaderMeta {
   StateCheckerType type = STATE_CHECKER_UNKNOWN;
   std::string key_seprator = "\t";
   std::string value_seprator = ",";
   std::string file_name;
   std::string redis_host;
   int redis_port;
   std::string redis_key;
   std::string redis_tm_key;
   int interval = 60;  // seconds
   int start_hour = 0;
   int end_hour = 24;

   std::string meta() {
     std::string meta_info;
     if (type == STATE_CHECKER_FILE) {
       meta_info = file_name;
     } else if (type == STATE_CHECKER_REDIS) {
       meta_info = redis_key + " " + redis_tm_key;
     }
     return meta_info; 
   }
};

class StateCheckerInterface {
 public:
  StateCheckerInterface()
      : tm_(0), check_tm_(0),
        start_hour_(0), end_hour_(24) {
  }

  StateCheckerInterface(int start_hour, int end_hour)
      : tm_(0), check_tm_(0),
        start_hour_(0), end_hour_(24) {
  }

  virtual ~StateCheckerInterface() {
  }

  virtual ReloaderState CheckState() = 0;

  virtual void UpdateState() {
    tm_ = check_tm_;
  }

  bool CheckTime() {
    int current_hour = GetCurrentHour();

    // LOG(INFO) << "current_hour:" << current_hour
              // << " start_hour:" << start_hour_
              // << " end_hour:" << end_hour_;

    if (current_hour >= start_hour_ && current_hour < end_hour_) {
      return true;
    } else {
      return false;
    }
  }

  static int ToDate(const time_t& unix_sec, struct tm* tm, int time_zone) {
    static const int kHoursInDay = 24;
    static const int kMinutesInHour = 60;
    static const int kDaysFromUnixTime = 2472632;
    static const int kDaysFromYear = 153;
    static const int kMagicUnkonwnFirst = 146097;
    static const int kMagicUnkonwnSec = 1461;
    tm->tm_sec = unix_sec % kMinutesInHour;
    int i = (unix_sec/kMinutesInHour);
    tm->tm_min = i % kMinutesInHour; //nn
    i /= kMinutesInHour;
    tm->tm_hour = (i + time_zone) % kHoursInDay; // hh
    tm->tm_mday = (i + time_zone) / kHoursInDay;
    int a = tm->tm_mday + kDaysFromUnixTime;
    int b = (a*4  + 3)/kMagicUnkonwnFirst;
    int c = (-b*kMagicUnkonwnFirst)/4 + a;
    int d =((c*4 + 3) / kMagicUnkonwnSec);
    int e = -d * kMagicUnkonwnSec;
    e = e/4 + c;
    int m = (5*e + 2)/kDaysFromYear;
    tm->tm_mday = -(kDaysFromYear * m + 2)/5 + e + 1;
    tm->tm_mon = (-m/10)*12 + m + 2 + 1;
    tm->tm_year = b*100 + d  - 6700 + (m/10) + 1900;
    return 0;
  }

  static int GetCurrentHour() {
    time_t now = time(NULL);
    struct tm tm;
    ToDate(now, &tm, 8);
    return tm.tm_hour;
  }

 protected:
  int64_t tm_;
  int64_t check_tm_;
  int start_hour_;
  int end_hour_;
};

class FileStateChecker : public StateCheckerInterface {
  public:
    explicit FileStateChecker(ReloaderMeta& reloader_meta)
      : StateCheckerInterface(reloader_meta.start_hour, reloader_meta.end_hour),
        file_name_(reloader_meta.file_name) {
      }

    ReloaderState CheckState() override {
      if (!CheckTime()) {
        return STATE_NOT_TIME;
      }
      struct stat curr_stat;
      if ((stat(file_name_.c_str(), &curr_stat) != 0)
          || ((curr_stat.st_mode & S_IFREG) == 0)) {
        return STATE_UNKNOWN;
      }
      if (curr_stat.st_mtime > tm_ ) {
        check_tm_ = curr_stat.st_mtime;
        return STATE_CHANGED;
      }
      return STATE_NOT_CHANGED;
    }

  private:
    std::string file_name_;
};

class RedisStateChecker : public StateCheckerInterface {
  public:
    RedisStateChecker(ReloaderMeta& reloader_meta)
      : StateCheckerInterface(reloader_meta.start_hour, reloader_meta.end_hour),
        reloader_meta_(reloader_meta) {
        redis_client_ = std::make_shared<RedisClient>(
            reloader_meta.redis_host, reloader_meta.redis_port);
        int status = redis_client_->Connect();
        if (status != 0) {
          LOG(ERROR) << "connect redis error in redis state checker meta:"
                     << reloader_meta.meta();
        }
      }

    ReloaderState CheckState() override {
      if (!CheckTime()) {
        return STATE_NOT_TIME;
      }
      if (redis_client_ == nullptr) {
        return STATE_UNKNOWN;
      }
      std::string redis_timestamp = redis_client_->get(
        reloader_meta_.redis_tm_key);

    int64_t tm = (int64_t)std::strtod(redis_timestamp.c_str(), NULL);

    // LOG(INFO) << "redis timestamp str:" << redis_timestamp
    //           << " tm:" << tm
    //           << " last_tm:" << tm_
    //           << " check_tm:" << check_tm_
    //           << " key: "
    //           << reloader_meta_.redis_tm_key;

    if (tm > tm_) {
      check_tm_ = tm;
      return STATE_CHANGED;
    }
    return STATE_NOT_CHANGED;
  }

 private:
  ReloaderMeta reloader_meta_;
  std::shared_ptr<RedisClient> redis_client_;
};

class StateCheckerFactory {
 public:
  static StateCheckerInterface* CreateStateChecker(
      ReloaderMeta& reloader_meta) {
    StateCheckerInterface* state_checker = NULL;
    switch(reloader_meta.type) {
      case STATE_CHECKER_FILE:
        state_checker = new FileStateChecker(reloader_meta);
        break;
      case STATE_CHECKER_REDIS:
        state_checker = new RedisStateChecker(reloader_meta);
        break;
      default:
        state_checker = NULL;
    }
    return state_checker;
  }
};

template <class T, class... Args>
class ReloaderEntry {
 public:
  explicit ReloaderEntry(ReloaderMeta reloader_meta)
      : reloader_meta_(reloader_meta) {
    _using_no = 1;
    _content[0] = _content[1] = NULL;
    state_checker_ = StateCheckerFactory::CreateStateChecker(reloader_meta_);
  }

  ~ReloaderEntry() {
    if (_content[0] != NULL) {
      delete _content[0];
      _content[0] = NULL;
    }
    if (_content[1] != NULL) {
      delete _content[1];
      _content[1] = NULL;
    }
    delete state_checker_;
  }

  int Load(Args... args) {
    if (state_checker_ == NULL) {
      LOG(FATAL) << "state check is null";
      return -1;
    }
    ReloaderState state = state_checker_->CheckState();
    if (state != STATE_CHANGED) {
      LOG(INFO) << "not loaded status:" << state
        << " meta:" << reloader_meta_.meta();
      return -1;
    }
    T* p = new (std::nothrow) T(args...);
    if (First(p) < 0) {
      delete p;
      LOG(ERROR) << "first load error meta:" << reloader_meta_.meta();
      return -1;
    }
    return 0;
  }
  int Reload(Args... args) {
    int ret = -1;
    if (state_checker_ == NULL) {
      return ret;
    }
    ReloaderState state = state_checker_->CheckState();
    if (state == STATE_CHANGED) {
      // LOG(INFO) << "reload status:" << state
                 // << " meta:" << reloader_meta_.meta();
      T* p = new (std::nothrow) T(args...);
      if (Second(p) < 0) {
        delete p;
        LOG(ERROR) << "reload error"
                   << " meta:" << reloader_meta_.meta();
      } else {
        LOG(INFO) << "reload successfully"
                  << " meta:" << reloader_meta_.meta();
        ret = 0;
      }
    } else {
      // LOG(INFO) << "not reloaded status:" << state
                 // << " meta:" << reloader_meta_.meta();
    }
    return ret;
  }

  T* GetContent() { return _content[_using_no]; }
  const T* GetContent() const { return GetContent(); }

 private:
  unsigned char _using_no;
  StateCheckerInterface* state_checker_;
  T* _content[2];
  std::unordered_map<std::string, std::string> params_;
  ReloaderMeta reloader_meta_;

 private:
  ReloaderEntry(const ReloaderEntry&) = delete;
  ReloaderEntry & operator=(const ReloaderEntry&) = delete;

  int First(T*& p) {
    if (p == NULL) { return -1; }
    if (p->Load() < 0) {
      return -1;
    }

    state_checker_->UpdateState();
    _content[_using_no] = p;
    return 0;
  }
  int Second(T*& p) {
    if (p == NULL) { return -1; }
    if (p->Load() < 0) {
      LOG(ERROR) << "second load error"
                 << " meta:" << reloader_meta_.meta();
      return -1;
    }

    state_checker_->UpdateState();
    int reload_no = 1 - _using_no;
    _content[reload_no] = p;

    RELOADER_MEMORY_BARRIER();
    __sync_synchronize();

    _using_no = reload_no;
    int previous_no = 1 - _using_no;
    if (NULL != _content[previous_no]) {
      sleep(kOldBufferWaitSeconds);
      delete _content[previous_no];
      _content[previous_no] = NULL;
    }

    malloc_trim(kMemSize);
    return 0;
  }
};  // ReloaderEntry

}  // namespace reloader
}  // namespace util
}  // namespace prediction

#endif  // RELOADER_RELOADER_ENTRY_H_

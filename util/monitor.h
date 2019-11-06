#ifndef PREDICTION_SERVER_UTIL_MONITOR_H_
#define PREDICTION_SERVER_UTIL_MONITOR_H_

// #include <stdlib.h>
// #include <pthread.h>
// #include <unistd.h>
#include <sys/time.h>
#include <boost/lockfree/queue.hpp>
#include "monitor_data.h"

namespace prediction {
namespace util {
namespace monitor {

#define kDefaultUpdateInterval 60 // seconds
#define kDefaultMaxQueueSize 65000

struct UpdatePara {
 public:
  std::string key;
  double val;
  Operation opertation;
  UpdatePara() {  }
  UpdatePara(std::string _key, double _val, Operation _op)
    : key(_key), val(_val), opertation(_op) {  }
};

typedef boost::lockfree::queue<UpdatePara*,
          boost::lockfree::capacity<kDefaultMaxQueueSize>>
            LockFreeQueue;

class Monitor {
 public:
  Monitor();
  virtual ~Monitor();

  int Init(const std::string& db_file,
      const std::string& monitor_file,
      long interval = kDefaultUpdateInterval);   
  int Start();
  void Dump(std::string* dump_str); 

  int Update(const std::string& key, double value, Operation op = ALL);

 public:
  static Monitor& GetInstance() {
    static Monitor instance;
    return instance;
  }
  static int avg(const std::string& key, double value);
  static int inc(const std::string& key, double value);
  static int max(const std::string& key, double value);
  static int min(const std::string& key, double value);


 private:
  int InitMonitorData(MonitorData*& monitor_data);

  static uint64_t GetFloorCurrentSeconds(int base) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t now = tv.tv_sec + tv.tv_usec / (1000 * 1000);
    // return (uint64_t)(floor(current / 1000));
    // uint64_t now = TimeUtils::GetCurrentTime() / 1000;
    if (base > 0) {
      return uint64_t(now / base) * base;
    } else {
      return now;
    }
  }

  void MonitorThread();
  void DumpThread();

 private: 
  MonitorData* monitor_data_;
  std::string mmap_file_;
  std::string lock_file_;
  bool init_flag_;

  LockFreeQueue queue_;

  long update_interval_;
  std::string monitor_status_file_;
  int last_update_minute_;
};

}  // namespace monitor
}  // namespace util
}  // namespace prediction

#endif  // PREDICTION_SERVER_UTIL_MONITOR_H_

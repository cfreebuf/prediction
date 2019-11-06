// CopyRight 2019 360. All rights reserved.
// File   time_utils.h
// Date   2019-10-28 15:20:18
// Brief

#ifndef PREDICTION_SERVER_UTIL_TIME_UTILS_H_
#define PREDICTION_SERVER_UTIL_TIME_UTILS_H_

#include <string>
#include <time.h>
#include <sys/time.h>
#include <unordered_map>

namespace prediction {
namespace util {

class TimeUtils {
 public:
  // Return current system time in seconds
  static int64_t DateToTimestamp(const std::string& date);
  // milliseconds
  static int64_t GetCurrentTime();
  static int64_t GetCurrentTimeRound();
  static void GetCurrentTimeTs(timespec * ts);
  static int GetCurrentMinute();
  static int GetCurrentHour();
  static int ToDate(const time_t& unix_sec, struct tm* tm, int time_zone);
  static void setJetLag();
  static int GetHour(int64_t timestamp);
  static int GetWeekday(int64_t timestamp);

 private:
  static int jet_lag; 
};  // TimeUtils

class TimeRecorder {
 public:
  TimeRecorder();
  virtual ~TimeRecorder();
  void StartTimer(const std::string& timer_name);
  void StopTimer(const std::string& timer_name);
  int64_t GetElapse(const std::string& timer_name);
  int64_t GetStartTime(const std::string& timer_name);
  int64_t GetStopTime(const std::string& timer_name);
  std::string ToString() const;

 private:
  std::unordered_map<std::string, int64_t> start_time_;
  std::unordered_map<std::string, int64_t> elapse_time_;
  std::unordered_map<std::string, int64_t> stop_time_;
};  // TimeRecorder

}  // namespace util
}  // namespace prediction

#endif  // PREDICTION_SERVER_UTIL_TIME_UTILS_H_

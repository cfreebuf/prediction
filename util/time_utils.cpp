// CopyRight 2019 360. All rights reserved.
// File   time_utils.cpp
// Date   2019-10-28 15:19:34
// Brief

#include "util/time_utils.h"
#include "util/string_utils.h"
#include <string.h>
#include <sstream>
#include <chrono>

namespace prediction {
namespace util {

int TimeUtils::jet_lag = 0;

// 2018-09-04 23:59:59
// Return seconds
int64_t TimeUtils::DateToTimestamp(const std::string& date) {
    struct tm stm;
    memset(&stm, 0, sizeof(stm));

    int y, m, d, h, min, s;

    std::vector<std::string> date_items;
    StringUtils::Split(date, ' ', date_items);
    if (date_items.size() != 2) {
        return -1;
    }

    std::vector<std::string> v;
    StringUtils::Split(date_items[0], '-', v);
    if (v.size() != 3) {
        return -1;
    }
    y = atoi(v[0].c_str());
    m = atoi(v[1].c_str());
    d = atoi(v[2].c_str());

    v.clear();
    StringUtils::Split(date_items[1], ':', v);
    if (v.size() != 3) {
        return -1;
    }
    h = atoi(v[0].c_str());
    min = atoi(v[1].c_str());
    s = atoi(v[2].c_str());

    stm.tm_year = y - 1900;
    stm.tm_mon = m - 1;
    stm.tm_mday = d;
    stm.tm_hour = h;
    stm.tm_min = min;
    stm.tm_sec = s;
    return (int64_t)mktime(&stm);
}

int TimeUtils::ToDate(const time_t& unix_sec,
                      struct tm* tm, int time_zone) {
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

int TimeUtils::GetCurrentHour() {
    time_t now = time(NULL);
    struct tm tm;
    ToDate(now, &tm, 8);
    return tm.tm_hour;
}

int64_t TimeUtils::GetCurrentTime() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int64_t TimeUtils::GetCurrentTimeRound() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000;
}

void TimeUtils::GetCurrentTimeTs(timespec* ts) {
    if (ts == NULL) { return; }
    clock_gettime(CLOCK_REALTIME, ts);
}

int TimeUtils::GetCurrentMinute() {
    time_t now = time(NULL);
    struct tm* tmp = localtime(&now); 
    return tmp->tm_min;
}

void TimeUtils::setJetLag() {
    time_t t1, t2;
    struct tm *tm_local, *tm_utc;

    time(&t1);
    t2 = t1;
    tm_local = localtime(&t1);
    t1 = mktime(tm_local);
    tm_utc = gmtime(&t2);
    t2 = mktime(tm_utc);
    jet_lag = (t1 - t2) / 3600;
}

int TimeUtils::GetHour(int64_t timestamp) {
    if (timestamp <= 0) {
        return -1;
    }
    auto ms = std::chrono::milliseconds(timestamp);
    auto tp = std::chrono::time_point<std::chrono::system_clock,
              std::chrono::milliseconds>(ms);
    auto tt = std::chrono::system_clock::to_time_t(tp);
    std::tm* now = std::gmtime(&tt);  // UTC
    // std::tm* now = std::localtime(&tt);  // Local
    // +24 to avoid negtive hour
    return (now->tm_hour + jet_lag + 24) % 24;
}

int TimeUtils::GetWeekday(int64_t timestamp) {
    if (timestamp <= 0) {
        return -1;
    }
    auto ms = std::chrono::milliseconds(timestamp);
    auto tp = std::chrono::time_point<std::chrono::system_clock,
             std::chrono::milliseconds>(ms);
    auto tt = std::chrono::system_clock::to_time_t(tp);
    std::tm* now = std::gmtime(&tt);
    // std::tm* now = std::localtime(&tt);
    if (jet_lag > 0) {
        now->tm_wday = now->tm_hour + jet_lag >= 24 ? (now->tm_wday + 1) % 7 : now->tm_wday;
    }
    else if (jet_lag < 0) {
        // +7 to avoid negtive week
        now->tm_wday = now->tm_hour + jet_lag <= 0 ? (now->tm_wday - 1 + 7) % 7 : now->tm_wday;
    }

    return now->tm_wday;
}

TimeRecorder::TimeRecorder() { }

TimeRecorder::~TimeRecorder() { }

void TimeRecorder::StartTimer(const std::string& timer_name) {
    start_time_[timer_name] = TimeUtils::GetCurrentTime();
}

void TimeRecorder::StopTimer(const std::string& timer_name) {
    auto it = start_time_.find(timer_name);
    if (it == start_time_.end()) {
        // ignore
    } else {
        int64_t start = it->second;
        int64_t current = TimeUtils::GetCurrentTime();
        elapse_time_[timer_name] = current - start;
        stop_time_[timer_name] = current;
    }
}

int64_t TimeRecorder::GetStartTime(const std::string& timer_name) {
    auto it = start_time_.find(timer_name);
    if (it == start_time_.end()) {
        return -1;
    } else {
        return it->second;
    }
}

int64_t TimeRecorder::GetStopTime(const std::string& timer_name) {
    auto it = stop_time_.find(timer_name);
    if (it == stop_time_.end()) {
        return -1;
    } else {
        return it->second;
    }
}

int64_t TimeRecorder::GetElapse(const std::string& timer_name) {
    auto it = elapse_time_.find(timer_name);
    if (it == elapse_time_.end()) {
        return -1;
    } else {
        return it->second;
    }
}

std::string TimeRecorder::ToString() const {
    auto it = elapse_time_.begin();
    std::string s;
    for (; it != elapse_time_.end(); it++) {
        s += it->first + "=" + std::to_string((long)it->second) + " ms, ";
    }

    return s;
}

}  // namespace util
}  // namespace prediction

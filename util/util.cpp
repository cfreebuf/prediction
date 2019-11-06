// CopyRight 2019 360. All rights reserved.
// File   util.cpp
// Date   2019-10-28 16:52:03
// Brief

#include <string.h>
#include <execinfo.h>  // backtrace
#include <fstream>
#include <random>
#include <thread>
#include <sstream>
#include <string>
#include "util.h"

// #include "glog/logging.h"
// #include "city.h"
// #include "common/common_gflags.h"
// #include <boost/math/distributions.hpp>

namespace prediction {
namespace util {

// uint64_t MakeHash(uint64_t key) { return key; };
// uint64_t MakeHash(const std::string& key) { return MAKE_HASH(key); };

// Return a random integer in the interval [a, b] 
int Random(int a, int b) {
    static std::default_random_engine e { std::random_device{  }() };
    static std::uniform_int_distribution<int> u;
    return u(e, decltype(u)::param_type(a, b));
}

double Random(double a, double b) {
    static std::default_random_engine e { std::random_device{  }() };
    static std::uniform_real_distribution<double> u;
    return u(e, decltype(u)::param_type(a, b));
}

#if 0
void PrintStacktrace() {
    int size = 16;
    void * array[16];
    int stack_num = backtrace(array, size);
    char ** stacktrace = backtrace_symbols(array, stack_num);
    for (int i = 0; i < stack_num; ++i) {
      LOG(INFO) << "bt:" << stacktrace[i];
    }
    free(stacktrace);
}
#endif

int CopyFile(std::string srcfile, std::string newfile) {
    std::ifstream in;
    std::ofstream out;
    in.open(srcfile.c_str());
    if (in.fail()) {
        // LOG_ERROR("open src file error:%s", srcfile.c_str());
        in.close();
        out.close();
        return -1;
    }
    out.open(newfile.c_str(), std::ios::trunc);
    if (out.fail()) {
        // LOG_ERROR("open dst file error:%s", newfile.c_str());
        out.close();
        in.close();
        return -1;
    } else {
        out << in.rdbuf();
        out.close();
        in.close();
        return 0;
    }
}

int SetThreadAffinity(std::thread& t, std::vector<int>& cores) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    for (auto core : cores) {
        CPU_SET(core, &cpuset);  // set affinity of io threads to cpu core 0
    }

    return pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);
}

int64_t GetTimestamp(const std::string& format, const std::string& time_str) {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));
    time_t tt;
    int64_t ts = 0;
    strptime(time_str.c_str(), format.c_str(), &tm);
    tt = mktime(&tm);
    ts = (int64_t)tt * 1000;
    return ts;
}

unsigned int random_char() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> dis(0, 255);
  return dis(gen);
}

std::string generate_hex(const unsigned int len) {
  std::stringstream ss;
  for (auto i = 0; i < len; i++) {
    const auto rc = random_char();
    std::stringstream hexstream;
    hexstream << std::hex << rc;
    auto hex = hexstream.str();
    ss << (hex.length() < 2 ? '0' + hex : hex);
  }
  return ss.str();
}

#if 0
double BetaSample(double& a, double& b) {
  //static std::uniform_real_distribution<double> unif(0,1);
  static std::default_random_engine engine;
  std::uniform_real_distribution<double> unif(0,1);
  double p = unif(engine);
  return boost::math::ibeta_inv(a, b, p);
}
#endif

#if 0
using google::protobuf::LogLevel;

LogStack log_stack;
bool pb_errno;

void PbLogHandler(LogLevel level, const char* filename, int line, const std::string& message) {
  // log_stack.push_back({level, filename, line, message});
  LogMessage msg = std::make_tuple(level, filename, line, message);
  log_stack.push_back(msg);
  pb_errno = true;
}

bool GetPbError(LogStack* pb_stack) {
  if (pb_errno && pb_stack) {
    pb_stack->assign(log_stack.begin(), log_stack.end());
  }

  log_stack.clear();
  bool old_errno = pb_errno;
  pb_errno = false;

  return old_errno;
}

void LogPbError() {
  // Protobuf error log
  LogStack pb_stack;
  if (GetPbError(&pb_stack)) {
    for (auto& m : pb_stack) {
      LOG_INFO("pb error level:%d, file:%s line:%d msg:%s",
          std::get<0>(m), std::get<1>(m).c_str(),
          std::get<2>(m), std::get<3>(m).c_str());
    }
  }

}
#endif

}  // namespace util
}  // namespace prediction

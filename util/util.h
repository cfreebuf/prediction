// Author lidongming
// Date   2017-03-22 17:02:53
// Brief

#ifndef PREDICTION_SERVER_UTIL_UTIL_H_
#define PREDICTION_SERVER_UTIL_UTIL_H_

#include <string>
#include <thread>
#include <codecvt>
#include <locale>
#include "string_utils.h"

namespace prediction {
namespace util {

extern void PrintStacktrace();

#define NUM2STR(num) StringUtils::Num2Str(num)

int CopyFile(std::string srcfile, std::string newfile);


// Return a random integer in the interval [a, b] 
extern int Random(int a, int b);

extern double Random(double a, double b);

extern int SetThreadAffinity(std::thread& t, std::vector<int>& cores);
extern int64_t GetTimestamp(const std::string& format, const std::string& time_str);

extern double BetaSample(double& a, double& b);

extern unsigned int random_char();
extern std::string generate_hex(const unsigned int len);

// extern int64_t MAKE_HASH(const std::string& key);
// extern int64_t MAKE_HASH(const char* key);

#if 0
typedef std::tuple<google::protobuf::LogLevel, std::string, int, std::string> LogMessage;
typedef std::list<LogMessage> LogStack;

extern LogStack log_stack;
extern bool pb_errno;

void PbLogHandler(google::protobuf::LogLevel level, const char* filename,
               int line, const std::string& message);
bool GetPbError(LogStack* pb_stack);
void LogPbError();
#endif

}  // namespace util
}  // namespace prediction

#endif  // PREDICTION_SERVER_UTIL_UTIL_H_

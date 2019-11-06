// CopyRight 2019 360. All rights reserved.
// File   string_utils.h
// Date   2019-10-28 17:06:13
// Brief

#ifndef PREDICTION_UTIL_STRING_UTILS_H_
#define PREDICTION_UTIL_STRING_UTILS_H_

#include <string>
#include <vector>

namespace prediction {
namespace util {

class StringUtils {
 public:
  static std::vector<std::string>& Split(const std::string & s,
      char delim, std::vector<std::string>& elems);
  static void Split(const char* str, char delim,
      std::vector<std::string>& output);
  static void Split(std::string& str, std::string& delim,
      std::vector<std::string>&output);
  static void Split(std::string& str, const char *delim,
      std::vector<std::string>& output);

  static void ReplaceAll(std::string& str, const std::string& from,
      const std::string& to);

  static void ToLower(std::string & str);

  static bool StartsWith(const std::string& fullstr,
      const std::string& prefix);
  static bool EndsWith(const std::string& fullstr,
      const std::string& ending);

  static void Trim(std::string& str);

  static std::string RandomString(int len);
  // static std::string UrlEncode(const std::string& str);
};

}  // namespace util
}  // namespace prediction

#endif  // PREDICTION_UTIL_STRING_UTILS_H_

// CopyRight 2019 360. All rights reserved.
// File   regex_util.h
// Date   2019-10-28 14:57:33
// Brief

#ifndef PREDICTION_SERVER_UTIL_REGEX_H_
#define PREDICTION_SERVER_UTIL_REGEX_H_

#include <string>
#include <regex>

namespace prediction {

class Regex {
 public:
  Regex(const char* pattern);
  Regex(const std::string& pattern);
  Regex(const std::string& pattern, bool is_sub_match);
  Regex(const Regex& other_regex);
  virtual ~Regex();

  Regex& operator = (const Regex& other_regex);

  bool Match(const std::string& str);
  bool Match(const char* str);

  bool IsInited() const { return _init; }

  const std::string& GetPattern() const { return _pattern; }

 private:
  bool _init;
  bool _is_sub_match;
  std::regex _reg;
  std::string _pattern;
};  // Regex

}  // namespace rankserver
#endif  // PREDICTION_SERVER_UTIL_REGEX_H_ 

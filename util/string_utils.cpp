// CopyRight 2019 360. All rights reserved.
// File   string_utils.cpp
// Date   2019-10-28 17:07:09
// Brief

#include <stdio.h>
#include <set>
#include <sstream>
#include <algorithm>
#include "util/string_utils.h"

namespace prediction {
namespace util {

std::vector<std::string>& StringUtils::Split(
    const std::string& s, char delim,
    std::vector<std::string>& elems) {
  Split(s.c_str(), delim, elems);
  return elems;
}

void StringUtils::Split(const char* str, char del,
    std::vector<std::string>& output) {
  output.clear();
  if (*str == '\0') { return; }

  const char* start = str;
  int i;

  for (i = 0; str[i] != '\0'; ++i) {
    if (str[i] == del) {
      output.emplace_back(start, &str[i]);
      start = &str[i + 1];
    }
  }

  if (start != &str[i]) {
    output.emplace_back(start, &str[i]);
  } else if (str[i - 1] == del) {
    // "a," should return [a, ""]
    output.emplace_back("");
  }

  return;
}

void StringUtils::Split(std::string& str, const char *delim,
    std::vector<std::string>& output) {
  std::string delim_str = delim;
  Split(str, delim_str, output);
}

void StringUtils::Split(std::string& str, std::string& delim,
    std::vector<std::string>&output) {
  output.clear();
  auto start = 0U;
  auto end = str.find(delim, start);
  while (end != std::string::npos) {
    output.push_back(str.substr(start, end-start));
    start = end + delim.size();
    end = str.find(delim, start);
  }

  if (start < str.size()) {
    output.push_back(str.substr(start));
  }
}

void StringUtils::ReplaceAll(std::string& str, const std::string& from,
    const std::string& to) {
  if (str.empty()) return;
  if (from.empty()) return;

  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();
  }
}

void StringUtils::ToLower(std::string& str) {
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

bool StringUtils::EndsWith(const std::string& fullstr,
    const std::string& ending) {
  if (ending.empty()) { return true; }
  if (fullstr.length() < ending.length()) { return false; }
  return (fullstr.substr(fullstr.length() - ending.length(), ending.length())
      == ending);
}

bool StringUtils::StartsWith(const std::string& fullstr,
    const std::string& prefix) {
  if (prefix.empty()) { return true; }
  if (fullstr.length() < prefix.length()) { return false; }
  return (fullstr.substr(0, prefix.length()) == prefix);
}

void StringUtils::Trim(std::string& str) {
  while (str.begin() != str.end() && iswspace(*str.begin())) {
    str.erase(str.begin());
  }

  auto it = str.end();
  while (it != str.begin() && iswspace(*--it)) {
    str.erase(it);
  }
}

std::string StringUtils::RandomString(int len) {
  static char CH[] = "_0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
  std::string s;
  s.reserve(len);
  for (int i = 0; i < len; ++i) {
    int index = rand()/(RAND_MAX/(sizeof(CH)-1));
    s[i] = CH[index];
  }

  return s;

}

#if 0
std::string StringUtils::UrlEncode(const std::string& str) {
  std::string buffer;
  char* p = curl_escape(str.c_str(), (int) str.size());
  if (!p) {
    return str;
  } else {
    buffer = p;
    curl_free(p);
  }

  return buffer;
}
#endif

}  // namespace util
}  // namespace prediction

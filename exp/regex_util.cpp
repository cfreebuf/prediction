// CopyRight 2019 360. All rights reserved.
// File   regex_util.cpp
// Date   2019-10-28 14:57:54
// Brief

#include "exp/regex_util.h"

namespace prediction {

Regex::Regex(const std::string & pattern): _init(false), _is_sub_match(false), _reg(pattern) {
    _pattern = pattern;
    if (_pattern.empty()) { return; }
    _init = true;
}

Regex::Regex(const char * pattern) : _init(false), _is_sub_match(false), _reg(pattern) {
    _pattern.assign(pattern);
    if (_pattern.empty()) { return; }
    _init = true;
}


Regex::Regex(const std::string& pattern, bool is_sub_match) : _init(false), _is_sub_match(true), _reg(pattern) {
    _pattern = pattern;
    if (_pattern.empty()) { return; }
    _init = true;
}

Regex::Regex(const Regex& other_regex) : _init(false) {
    if (!other_regex.IsInited()) { return; }

    _is_sub_match = other_regex._is_sub_match;
    _pattern = other_regex.GetPattern();
    _reg = _pattern.c_str();
    _init = true;
}

Regex& Regex::operator=(const Regex& other_regex) {
    if (this != &other_regex) {
        if (other_regex.IsInited()) {
            _is_sub_match = other_regex._is_sub_match;
            _pattern = other_regex.GetPattern();
            _reg = _pattern.c_str();
            _init = true;
        }
    }
    return *this;
}

Regex::~Regex() { _init = false; }

bool Regex::Match(const std::string& str) { return Match(str.c_str()); }

bool Regex::Match(const char* str) {
    bool match = false;
    if (_init) {
      if (_is_sub_match) {
        match = std::regex_search(str, _reg);
      } else {
        match = std::regex_match(str, _reg);
      }
    }

    return match;
}

}  // namespace prediction

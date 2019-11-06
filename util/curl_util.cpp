// CopyRight 2019 360. All rights reserved.
// File   curl_util.cpp
// Date   2019-11-06 01:24:16
// Brief

#include <curl/curl.h>
#include "prediction/util/curl_util.h"

namespace prediction {
namespace util {

std::string urldecode(const std::string& encoded) {
  CURL *curl = curl_easy_init();
  int out_length;
  char* raw_res = curl_easy_unescape(curl, encoded.c_str(), encoded.length(), &out_length);
  std::string res(raw_res, raw_res + out_length);
  curl_free(raw_res);
  curl_easy_cleanup(curl);
  return res;
}

}  // namespace util
}  // namespace prediction

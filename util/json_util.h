// CopyRight 2019 360. All rights reserved.
// File   json_util.h
// Date   2019-11-05 21:06:25
// Brief

#ifndef PREDICTION_UTIL_JSON_UTIL_H_
#define PREDICTION_UTIL_JSON_UTIL_H_

#include "prediction/third_party/rapidjson/writer.h"
#include "prediction/third_party/rapidjson/document.h"
#include "prediction/third_party/rapidjson/rapidjson.h"
#include "prediction/third_party/rapidjson/stringbuffer.h"

namespace prediction {

#define JSON_STRING(_str_) rapidjson::StringRef(_str_.c_str())

}  // namespace prediction

#endif  // PREDICTION_UTIL_JSON_UTIL_H_

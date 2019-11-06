// CopyRight 2019 360. All rights reserved.
// File   status.cpp
// Date   2019-11-06 10:46:45
// Brief

#include <stdio.h>
#include <assert.h>
#include <string>
#include "prediction/util/status.h"

namespace prediction {
namespace util {

Status::Status(error::Code code, std::string msg) {
  assert(code != error::OK);
  state_ = std::unique_ptr<State>(new State);
  state_->code = code;
  state_->msg = msg;
}

const std::string& Status::empty_string() {
  static std::string* empty = new std::string;
  return *empty;
}

void Status::SlowCopyFrom(const State* src) {
  if (src == nullptr) {
    state_ = nullptr;
  } else {
    state_ = std::unique_ptr<State>(new State(*src));
  }
}

std::string Status::ToString() const {
  if (state_ == nullptr) {
    return "OK";
  } else {
    char tmp[30];
    const char* type;
    switch (code()) {
      case error::HTTP_QUERY_PARSE_ERROR:
        type = "HTTP_QUERY_PARSE_ERROR";
        break;
      case error::QUERY_PARAM_ERROR:
        type = "QUERY_PARAM_ERROR";
        break;
      case error::DOWNLOAD_IMAGE_ERROR:
        type = "DOWNLOAD_IMAGE_ERROR";
        break;
      case error::NO_INPUT_IMAGE:
        type = "NO_INPUT_IMAGE";
        break;
      case error::READ_IMAGE_ERROR:
        type = "READ_IMAGE_ERROR";
        break;
      case error::FACE_DETECTION_PREDICT_ERROR:
        type = "FACE_DETECTION_PREDICT_ERROR";
        break;
      case error::LOGO_DETECTION_PREDICT_ERROR:
        type = "LOGO_DETECTION_PREDICT_ERROR";
        break;
      case error::READ_TENSOR_FROM_IMAGE_ERROR:
        type = "READ_TENSOR_FROM_IMAGE_ERROR";
        break;
      case error::RUNNING_MODEL_ERROR:
        type = "RUNNING_MODEL_ERROR";
        break;
      case error::FACE_DETECTION_PNET_ERROR:
        type = "FACE_DETECTION_PNET_ERROR";
        break;
      case error::FACE_DETECTION_RNET_ERROR:
        type = "FACE_DETECTION_RNET_ERROR";
        break;
      case error::FACE_DETECTION_ONET_ERROR:
        type = "FACE_DETECTION_ONET_ERROR";
        break;
      default:
        snprintf(tmp, sizeof(tmp), "Unknown code(%d)",
                 static_cast<int>(code()));
        type = tmp;
        break;
    }
    std::string result(type);
    result += ": ";
    result += state_->msg;
    return result;
  }
}

std::ostream& operator<<(std::ostream& os, const Status& x) {
  os << x.ToString();
  return os;
}

}  // namespace util
}  // namespace prediction

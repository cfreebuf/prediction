// CopyRight 2019 360. All rights reserved.
// File   error_codes.h
// Date   2019-11-06 10:19:44
// Brief

#ifndef PREDICTION_UTIL_ERROR_CODES_H_
#define PREDICTION_UTIL_ERROR_CODES_H_

namespace prediction {
namespace error {

enum Code {
  OK = 0,
  HTTP_QUERY_PARSE_ERROR,
  QUERY_PARAM_ERROR,
  DOWNLOAD_IMAGE_ERROR,
  NO_INPUT_IMAGE,
  READ_IMAGE_ERROR,
  FACE_DETECTION_PREDICT_ERROR,
  LOGO_DETECTION_PREDICT_ERROR,
  READ_TENSOR_FROM_IMAGE_ERROR,
  RUNNING_MODEL_ERROR,
  FACE_DETECTION_PNET_ERROR,
  FACE_DETECTION_RNET_ERROR,
  FACE_DETECTION_ONET_ERROR,
};

}  // namespace error
}  // namespace prediction

#endif  // PREDICTION_UTIL_ERROR_CODES_H_

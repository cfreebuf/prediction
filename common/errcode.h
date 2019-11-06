// File   errcode.h
// Author lidongming
// Date   2019-01-09 17:38:39
// Brief

#ifndef PREDICTION_SERVER_COMMON_ERRORCODE_H_
#define PREDICTION_SERVER_COMMON_ERRORCODE_H_

namespace prediction {

// WARNING:错误码需小于0
enum PredictErrorCode {
  PREDICT_NO_MODEL = -1,
  PREDICT_NO_FEATURES = -2,
  PREDICT_INVALID_FEATURES = -3,
  PREDICT_UNSUPPORT_PREDICT_TYPE = -4,
  PREDICT_TF_TIMEOUT = -5,
  PREDICT_INVALID_REQUEST = -6,
  PREDICT_OK = 0,
};

}

#endif

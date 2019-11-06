// File   base_handler.h
// Author lidongming
// Date   2018-09-25 22:28:18
// Brief

#ifndef PREDICTION_SERVER_CHAIN_BASE_HANDLER_H_
#define PREDICTION_SERVER_CHAIN_BASE_HANDLER_H_

#include "server/request.h"
#include "server/result.h"

namespace prediction {

class BaseHandler {
 public:
  BaseHandler() {  }
  virtual ~BaseHandler() {  }
  virtual int Handle(Request* req, Result* res) = 0;
};  // BaseHandler

}  // namespace prediction

#endif  // PREDICTION_SERVER_CHAIN_BASE_HANDLER_H_

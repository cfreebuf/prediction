// File   request_processor.h
// Author lidongming
// Date   2018-09-25 21:37:37
// Brief

#ifndef PREDICTION_SERVER_CHAIN_REQUEST_PROCESSOR_H_
#define PREDICTION_SERVER_CHAIN_REQUEST_PROCESSOR_H_

#include "server/request.h"
#include "exp/multi_layer_exp_handler.h"

namespace prediction {

class RequestProcessor {
 public:
  RequestProcessor();
  virtual ~RequestProcessor();
  virtual int Process(Request* req);

  const std::string GetName() const { return "rank_request_processor"; }

  int StartReloadThread();

  int ReloadExpConf(const std::string& conf_path);

 private:
  MultiLayerExpHandler exp_handler_;
};

}  // RequestProcessor

#endif  // PREDICTION_SERVER_CHAIN_REQUEST_PROCESSOR_H_

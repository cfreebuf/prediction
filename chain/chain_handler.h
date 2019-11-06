// File   chain_handler.h
// Author lidongming
// Date   2018-09-26 11:52:26
// Brief

#ifndef PREDICTION_SERVER_CHAIN_CHAIN_HANDLER_H_
#define PREDICTION_SERVER_CHAIN_CHAIN_HANDLER_H_

#include <vector>

#include "prediction/chain/base_handler.h"
#include "prediction/chain/request_processor.h"
#include "prediction/chain/chain_factory.h"

class ChainFactory;

namespace prediction {

class ChainHandler : public BaseHandler {
 public:
  ChainHandler(ChainFactory* chain_factory);
  virtual ~ChainHandler();
  virtual int Handle(Request* req, Result* res);

  int RegisterRequestProcessor(RequestProcessor* p);
  int UnRegisterRequestProcessor(RequestProcessor* p);

 private:
  std::vector<RequestProcessor*> request_processor_;
  ChainFactory* chain_factory_;
};

}  // namespace prediction

#endif  // PREDICTION_SERVER_CHAIN_CHAIN_HANDLER_H_

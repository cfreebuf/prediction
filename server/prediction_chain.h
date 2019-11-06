// CopyRight 2019 360. All rights reserved.
// File   prediction_chain.h
// Date   2019-10-31 10:48:04
// Brief

#ifndef PREDICTION_SERVER_PREDICTION_CHAIN_H_
#define PREDICTION_SERVER_PREDICTION_CHAIN_H_

namespace prediction {

class HandlerEntry;
class ChainHandler;
class ChainFactory;
class FactoryContext;
class RequestProcessor;
class Request;
class Result;

class PredictionChain {
 public:
   PredictionChain();
   ~PredictionChain();

 private:
  FactoryContext* factory_context_{nullptr};
  ChainFactory* chain_factory_{nullptr};
  HandlerEntry* handler_entry_{nullptr};
  ChainHandler* chain_handler_{nullptr};
  RequestProcessor* request_processor_{nullptr};
}; // PredictionChain

}  // namespace prediction
#endif  // PREDICTION_SERVER_PREDICTION_CHAIN_H_

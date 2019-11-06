// CopyRight 2019 360. All rights reserved.
// File   prediction_chain.cpp
// Date   2019-10-31 10:51:35
// Brief

#include "prediction/server/prediction_chain.h"
#include "prediction/common/common_gflags.h"
#include "prediction/chain/chain_handler.h"
#include "prediction/chain/chain_factory.h"
#include "prediction/chain/handler_entry.h"
#include "prediction/chain/request_processor.h"
#include "prediction/chain/factory_context.h"
#include "prediction/util/logging.h"

namespace prediction {

PredictionChain::PredictionChain() {
  // Init chain factory
  factory_context_ = new FactoryContext();
  chain_factory_ = new ChainFactory(factory_context_);

  if (chain_factory_->LoadConfig(FLAGS_chain_conf) != 0) {
    LOG(FATAL) << "init chain factory error:" << FLAGS_chain_conf;
    return;
  }

  handler_entry_ = new HandlerEntry();
  chain_handler_  = new ChainHandler(chain_factory_);
  request_processor_ = new RequestProcessor();
  request_processor_->StartReloadThread();
  chain_handler_->RegisterRequestProcessor(request_processor_);
  handler_entry_->set_default_handler(chain_handler_);
  LOG(INFO) << "Prediction Chain inited successfully";
}

PredictionChain::~PredictionChain() {
  if (chain_factory_) {
    delete chain_factory_;
  }
  if (request_processor_) {
    delete request_processor_;
  }
}

}  // namespace prediction

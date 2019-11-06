// CopyRight 2019 360. All rights reserved.
// File   prediction.h
// Date   2019-10-31 10:26:08
// Brief

#ifndef PREDICTION_SERVER_PREDICTION_SERVER_H_
#define PREDICTION_SERVER_PREDICTION_SERVER_H_

#include <folly/Memory.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/Unistd.h>
#include <proxygen/httpserver/RequestHandlerFactory.h>
#include "prediction/common/common_define.h"
#include "prediction/server/prediction_stats.h"
#include "prediction/server/prediction_chain.h"
#include "prediction/server/prediction_handler.h"

namespace prediction {

using proxygen::RequestHandler;
using proxygen::HTTPMessage; 

class PredictionServer {
 public:
  PredictionServer() {}
  ~PredictionServer() {}
		
  void Start();

 private:
  // PREDICTION_DISALLOW_COPN_AND_ASSIGN(PredictionServer);
};  // PredictionServer

class PredictionHandlerFactory : public proxygen::RequestHandlerFactory {
 public:
  explicit PredictionHandlerFactory() {
    LOG(INFO) << "PredictionHandlerFactory inited";
  }

  void onServerStart(folly::EventBase*) noexcept override {
    LOG(INFO) << "Server started";
    stats_.reset(new PredictionStats);
  }

  void onServerStop() noexcept override {
    LOG(INFO) << "Server stopped";
    stats_.reset();
  }

  RequestHandler* onRequest(RequestHandler* handler, HTTPMessage* headers) noexcept override {
    return new PredictionHandler(stats_.get(), chain_.get());
  }

 private:
  folly::ThreadLocalPtr<PredictionStats> stats_;
  folly::ThreadLocalPtr<PredictionChain> chain_;
};

}  // namespace prediction
#endif  // PREDICTION_SERVER_PREDICTION_SERVER_H_

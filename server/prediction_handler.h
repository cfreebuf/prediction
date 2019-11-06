// CopyRight 2019 360. All rights reserved.
// File   prediction_handler.h
// Date   2019-11-06 16:59:23
// Brief

#ifndef PREDICTION_SERVER_SERVER_PREDICTION_HANDLER_H_
#define PREDICTION_SERVER_SERVER_PREDICTION_HANDLER_H_

#include <memory>
#include <folly/Memory.h>
#include <proxygen/httpserver/RequestHandler.h>
#include "prediction/util/status.h"

namespace proxygen {
class ResponseHandler;
}

namespace prediction {

class PredictionStats;
class PredictionChain;
class Request;
class Response;

class PredictionHandler : public proxygen::RequestHandler {
 public:
  explicit PredictionHandler(PredictionStats* stats, PredictionChain* chain);
  // PredictionHandler::PredictionHandler() {}
  void onRequest(std::unique_ptr<proxygen::HTTPMessage> headers) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> body) noexcept override;
  void onEOM() noexcept override;
  void onUpgrade(proxygen::UpgradeProtocol proto) noexcept override;
  void requestComplete() noexcept override;
  void onError(proxygen::ProxygenError err) noexcept override;

  util::Status BuildRequest(std::map<std::string, std::string>& headers_map,
                      Request* request);
  util::Status FaceDetectionHandler(Request* request, Response* response);
  util::Status LogoDetectionHandler(Request* request, Response* response);

 private:
  PredictionStats* const stats_{nullptr};
  PredictionChain* const chain_{nullptr};
  std::unique_ptr<folly::IOBuf> body_;

};  // PredictionHandler

}  // namespace prediction
#endif  // PREDICTION_SERVER_SERVER_PREDICTION_HANDLER_H_

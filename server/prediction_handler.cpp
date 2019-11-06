// CopyRight 2019 360. All rights reserved.
// File   prediction_handler.cpp
// Date   2019-10-29 12:14:17
// Brief

#include <proxygen/httpserver/RequestHandler.h>
#include <proxygen/httpserver/ResponseBuilder.h>

#include "prediction/util/status.h"
#include "prediction/util/curl_util.h"
#include "prediction/util/image_util.h"
#include "prediction/common/common_gflags.h"

#include "prediction/server/request.h"
#include "prediction/server/response.h"
#include "prediction/server/prediction_handler.h"
#include "prediction/server/prediction_stats.h"
#include "prediction/server/prediction_chain.h"
// #include "prediction/server/tensorflow_serving_adapter.h"

#include "prediction/module/face_detection_module.h"
#include "prediction/module/logo_detection_module.h"

using namespace proxygen;

namespace prediction {

PredictionHandler::PredictionHandler(PredictionStats* stats, PredictionChain* chain)
  : stats_(stats), chain_(chain) {}

util::Status PredictionHandler::BuildRequest(
    std::map<std::string, std::string>& headers_map, Request* request) {
  return request->Parse(headers_map["query"]);
}

util::Status PredictionHandler::FaceDetectionHandler(Request* request,
    Response* response) {
  LOG(INFO) << "Request face_detection";
  std::string image_url;
  if (request->GetStringParamFromJson("image_url", &image_url) != 0) {
    return util::Status(error::QUERY_PARAM_ERROR,
                        "Param image_url not set in http query");
  }

  std::string image_file = FLAGS_face_detection_req_path + request->rid() + ".jpg";

  util::Status status = DownloadImageFromURL(image_url, image_file);
  if (!status.ok()) {
    return status;
  }

  request->AddMember("input_image_path", image_file);
  request->Parse();
  LOG(INFO) << request->ToString();

  FaceDetectionModule module(FLAGS_mtcnn_model);
  status = module.Predict(*request, response);
  if (!status.ok()) {
    LOG(WARNING) << "[RESPONSE] rid:" << request->rid() << " status: "
                 << status.ToString() << " " << response->ToString();
  } else {
    LOG(INFO) << "[RESPONSE] rid:" << request->rid() << " status: "
              << status.ToString() << " " << response->ToString();
  }
  return status;
}

util::Status PredictionHandler::LogoDetectionHandler(Request* request,
    Response* response) {
  LOG(INFO) << "Request logo_detection";

  std::string image_url;
  if (request->GetStringParamFromJson("image_url", &image_url) != 0) {
    return util::Status(error::QUERY_PARAM_ERROR,
                        "Param image_url not set in http query");
  }
  std::string image_file = FLAGS_logo_detection_req_path + request->rid() + ".jpg";

  util::Status status = DownloadImageFromURL(image_url, image_file);
  if (!status.ok()) {
    return status;
  }

  request->AddMember("input_image_path", image_file);
  request->Parse();
  LOG(INFO) << request->ToString();

  LogoDetectionModule module(FLAGS_logo_model);
  status = module.Predict(*request, response);
  if (!status.ok()) {
    LOG(WARNING) << "[RESPONSE] rid:" << request->rid() << " status: "
                 << status.ToString() << " " << response->ToString();
  } else {
    LOG(INFO) << "[RESPONSE] rid:" << request->rid() << " status: "
              << status.ToString() << " " << response->ToString();
  }
  return status;
}

void PredictionHandler::onRequest(std::unique_ptr<HTTPMessage> headers) noexcept {
  // Skip request of getting facicon.ico
  if (headers->getPathAsStringPiece() == "/favicon.ico") {
    return;
  }

  stats_->recordRequest();

  // Only support GET
  if (headers->getMethod() != HTTPMethod::GET) {
    LOG(WARNING) << "Unsupported request method:" << headers->getMethodString();
    ResponseBuilder(downstream_)
        .status(400, "Bad Method")
        .body("Only GET method supported")
        .sendWithEOM();
    return;
  }

  // Parse query
  std::string query = headers->getQueryString();
  std::map<std::string, std::string> headers_map;

  std::vector<std::string> headers_list;
  folly::split('&', query, headers_list);
  for (const auto& header_pair : headers_list) {
    std::vector<std::string> nv;
    folly::split('=', header_pair, nv);
    if (nv.empty() || nv[0].empty()) {
      continue;
    }
    std::string value("");
    if (nv.size() > 1) {
      // value = nv[1];
      value = std::move(util::urldecode(nv[1]));
    }
    headers_map[nv[0]] = value;
  }

  // Check if query exists
  if (headers_map.find("query") == headers_map.end()
      || headers_map["query"].empty()) {
    LOG(WARNING) << "Invalid request, no query found";
    ResponseBuilder(downstream_)
        .status(400, "Invalid request, no query found")
        .body("Invalid request, no query found")
        .sendWithEOM();
    return;
  }

#if 0
  // Check if query is encoded
  bool encoded = (headers_map.find("encoded") != headers_map.end()
      && headers_map["encoded"] == "1") ? true : false;

  if (encoded) {
    for (auto& kv : headers_map) {
      headers_map[kv.first] = util::urldecode(kv.second);
    }
  }
#endif

  util::Status status;

  Request request;
  Response response;
  std::string rid = request.rid();
  response.set_rid(rid);

  status = BuildRequest(headers_map, &request);
  if (!status.ok()) {
   LOG(WARNING) << "Request failed, path:"
                 << folly::to<std::string>(headers->getPathAsStringPiece())
                 << " error_code:" << status.code() << " error_msg:"
                 << status.ToString();
    ResponseBuilder(downstream_)
      .status(500, "Request failed")
      .body(status.ToString())
      .sendWithEOM();
    return;
  }

  // Parse path
  if (headers->getPathAsStringPiece() == "/face_detection") {
    status = FaceDetectionHandler(&request, &response);
    LOG(INFO) << "FaceDetection response:" << response.json_response();
  } else if (headers->getPathAsStringPiece() == "/logo_detection") {
    status = LogoDetectionHandler(&request, &response);
  } else {
    LOG(WARNING) << "Unsupported request path:"
                 << folly::to<std::string>(headers->getPathAsStringPiece());
    ResponseBuilder(downstream_)
      .status(500, "Unsupported request path")
      .body("Unsupported request path")
      .sendWithEOM();
    return;
  }

  if (!status.ok()) {
    LOG(WARNING) << "Request failed, path:"
                 << folly::to<std::string>(headers->getPathAsStringPiece())
                 << " error_code:" << status.code() << " error_msg:"
                 << status.ToString();
    ResponseBuilder(downstream_)
      .status(500, "Request failed")
      .body(status.ToString())
      .sendWithEOM();
    return;
  }
  LOG(INFO) << "Request completed successfully, response:"
            << response.json_response();
  bool render_html = (headers_map.find("render_html") != headers_map.end()
                        && headers_map["render_html"] == "1") ? true : false;

  bool process_status = response.is_success();
  if (process_status) {
    if (render_html) {
      std::string result_image_url;
      std::string res;
      if (response.GetStringParamFromJson("image", &result_image_url) == 0) {
        res = "<div><p>Result</p><img src=\"" + result_image_url
          + "\" alt=\"result\"/></div>";
      } else {
        res = "<div><p>Invalid Result, No image url</p></div>";
      }
      ResponseBuilder(downstream_)
        .status(200, "OK")
        .body(std::move(res))
        .sendWithEOM();
    } else {
      ResponseBuilder(downstream_)
        .status(200, "OK")
        .body(std::move(response.json_response()))
        .sendWithEOM();
    }
  } else {
    ResponseBuilder(downstream_)
      .status(200, "No Content")
      .body(std::move(response.json_response()))
      .sendWithEOM();
  }
}

void PredictionHandler::onBody(std::unique_ptr<folly::IOBuf> body) noexcept {
  LOG(INFO) << "PredictionHandler onBody";
  if (body_) {
    body_->prependChain(std::move(body));
  } else {
    body_ = std::move(body);
  }
}

void PredictionHandler::onEOM() noexcept {
}

void PredictionHandler::onUpgrade(UpgradeProtocol protocol) noexcept {
  // handler doesn't support upgrades
}

void PredictionHandler::requestComplete() noexcept {
  delete this;
}

void PredictionHandler::onError(ProxygenError err) noexcept {
  LOG(INFO) << "PredictionHandler onError";
  delete this;
}

#if 0
void PredictionHandler::Predict(PredictResponse& response, const PredictRequest& request) {
  LOG(INFO) << "Start predict rid:" << request.rid;
  monitor::Monitor::inc("qpm", 1);

  // Timers
  TimeRecorder timer;
  timer.StartTimer(kPredictLatency);

  Request req;
  Result res;
  req.set_request(&request);
  res.set_response(&response);

  util::Status status = req.Parse(request);
  if (!status.ok()) {
    response.err_code = PREDICT_INVALID_REQUEST;
    return;
  }

  timer.StopTimer(kPredictLatency);

  int64_t latency = timer.GetElapse(kPredictLatency);
  int64_t docs_size = request.__isset.docs ? request.docs.size() : 0;

  monitor::Monitor::avg("avg_latency", latency);
  monitor::Monitor::max("max_latency", latency);

  monitor::Monitor::inc("count_of_model_" + req.Get("prediction_id"), 1);
  monitor::Monitor::avg("latency_of_model_" + req.Get("prediction_id"), timer.GetElapse(kPredictLatency));
  if (response.err_code != 0) {
    monitor::Monitor::inc("err_of_model_" + req.Get("prediction_id"), 1);
  }

  LOG(INFO) << "[PREDICT] rid:" << request.rid
            << " latency:" << timer.GetElapse(kPredictLatency)
            << " request_time:" << timer.GetStartTime(kPredictLatency)
            << " response_time:" << timer.GetStopTime(kPredictLatency)
            << " score_latency:" << req.Get("score_latency", "-1")
            << " prediction_id:" << req.Get("prediction_id")
            << " predict_type:" << req.Get("predict_type")
            << " err_code:" << response.err_code;

  if (latency > 100) { monitor::Monitor::inc("latency_100ms", 1); }
  else if (latency > 50) { monitor::Monitor::inc("latency_50ms", 1); }
  else if (latency > 30) { monitor::Monitor::inc("latency_30ms", 1); }
  else if (latency > 20) { monitor::Monitor::inc("latency_20ms", 1); }
  else { monitor::Monitor::inc("latency_less_20ms", 1); }

}
#endif

// void PredictionHandler::Process(Request* req, Result* res) {
    // handler_entry_->Handle(req, res);
// }

}  // namespace prediction

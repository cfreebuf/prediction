// CopyRight 2019 360. All rights reserved.
// File   response.h
// Date   2019-11-05 19:04:45
// Brief

#ifndef PREDICTION_SERVER_RESPONSE_H_
#define PREDICTION_SERVER_RESPONSE_H_

#include <memory>
#include <string>
#include "prediction/third_party/rapidjson/document.h"

namespace prediction {

class Response {
 public:
  Response() {
    res_json_doc_ = std::make_unique<rapidjson::Document>();
    res_json_doc_->SetObject();
  }
  ~Response() {}

  void AddMember(const char* key, const std::string& value);
  void AddMember(std::string key, const std::string& value);

  std::unique_ptr<rapidjson::Document>* mutable_res_json_doc() {
    return &res_json_doc_;
  }

  const std::unique_ptr<rapidjson::Document>& res_json_doc() const {
    return res_json_doc_;
  }

  std::string json_response();
  std::string ToString();

  void set_rid(const std::string& rid) {
    rid_ = rid;
    AddMember("rid", rid_);
  }

  int GetStringParamFromJson(std::string key, std::string* value);

  bool is_success() { return is_success_; }
  void set_success(bool v) { is_success_ = v; }

 private:
  std::unique_ptr<rapidjson::Document> res_json_doc_{nullptr};
  std::string rid_;
  bool is_success_{false};
};  // Response

}  // namespace prediction
#endif  // PREDICTION_SERVER_RESPONSE_H_

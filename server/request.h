// CopyRight 2019 360. All rights reserved.
// File   request.h
// Date   2019-11-05 18:35:31
// Brief

#ifndef PREDICTION_SERVER_FRAMEWORK_REQUEST_H_
#define PREDICTION_SERVER_FRAMEWORK_REQUEST_H_

#include <map>
#include <vector>
#include <string>
#include <memory>
#include "prediction/third_party/rapidjson/document.h"
#include "prediction/util/status.h"

namespace prediction {

class Request {
 public:
  Request();
  ~Request() {}

  void Parse();
  util::Status Parse(const std::string& body);
  void ParseParams();

  void AddMember(std::string key, std::string value);
  const std::string ToString() const;

    // Getters
  const std::string& Get(const std::string& key) const {
    auto it = params_.find(key);
    if (it != params_.end()) { return it->second; }
    static const std::string empty_string = "";
    return empty_string;
  }

  std::string Get(const std::string& key, const std::string& default_value) {
    const std::string& v = Get(key);
    if (v.empty()) return default_value;
    else return v;
  }

  int Get(const std::string& key, int default_value) {
    const std::string& v = Get(key);
    if (v.empty()) { return default_value; }
    return atoi(v.c_str());
  }

  double Get(const std::string& key, double default_value) {
    const std::string& v = Get(key);
    if (v.empty()) { return default_value; }
    return atof(v.c_str());
  }

  void Set(const std::string& key, const std::string& value) {
    params_[key] = value;
  }

  void Set(const std::string& key, int64_t value) {
    params_[key] = std::to_string(value);
  }

  const std::string rid() const { return rid_; }

  std::unique_ptr<rapidjson::Document>* mutable_req_json_doc() {
    return &req_json_doc_;
  }

  const std::unique_ptr<rapidjson::Document>& req_json_doc() const {
    return req_json_doc_;
  }

  int GetStringParamFromJson(std::string key, std::string* value);

 private:
  std::map<std::string, std::string> params_;
  std::string rid_;
  std::unique_ptr<rapidjson::Document> req_json_doc_{nullptr};
};  // Request

}  // namespace prediction
#endif  // PREDICTION_SERVER_FRAMEWORK_REQUEST_H_

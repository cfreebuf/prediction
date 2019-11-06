// CopyRight 2019 360. All rights reserved.
// File   request.cpp
// Date   2019-10-29 12:14:44
// Brief

#include "prediction/server/request.h"
#include "prediction/common/common_gflags.h"
#include "prediction/common/common_define.h"
#include "prediction/util/util.h"
#include "prediction/util/status.h"
#include "prediction/util/logging.h"
#include "prediction/util/json_util.h"
#include "prediction/util/time_utils.h"

namespace prediction {

Request::Request() {
  rid_ = util::generate_hex(32);
  params_["rid"] = rid_;
  req_json_doc_ = std::make_unique<rapidjson::Document>();
  req_json_doc_->SetObject();
  LOG(INFO) << "Receive request rid:" << rid_;
}

void Request::Parse() {
  ParseParams();
}

util::Status Request::Parse(const std::string& body) {
  req_json_doc_->Parse(body.c_str());
  if (req_json_doc_->HasParseError()) {
    LOG(WARNING) << "Parse request error, http query is not invalid json, rid:"
                 << rid_ << " str:" << body;
    return util::Status(error::HTTP_QUERY_PARSE_ERROR, "HTTP query is not invalid json");
  }

  ParseParams();
  return util::Status::OK();
}

void Request::ParseParams() {
  rapidjson::Document::AllocatorType& allocator = req_json_doc_->GetAllocator();
  for (const auto& kv : params_) {
    if (kv.first.empty()) {
      continue;
    }
    req_json_doc_->AddMember(JSON_STRING(kv.first), JSON_STRING(kv.second), allocator);
  }
}

const std::string Request::ToString() const {
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
  req_json_doc_->Accept(writer);
  const std::string request_type_str = "REQUEST:";
  return request_type_str + std::string(buffer.GetString());
}

void Request::AddMember(std::string key, std::string value) {
  params_[key] = value;
}

int Request::GetStringParamFromJson(std::string key, std::string* value) {
  if (req_json_doc_->HasMember(key.c_str())
      && (*req_json_doc_)[key.c_str()].IsString()) {
    *value = std::string((*req_json_doc_)[key.c_str()].GetString());
    return 0;
  } else {
    LOG(WARNING) << "Key not found in request, rid:" << rid_ << " key:" << key;
    return -1;
  }
}

}  // namespace prediction

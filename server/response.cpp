// CopyRight 2019 360. All rights reserved.
// File   response.cpp
// Date   2019-11-05 21:01:36
// Brief

#include "prediction/server/response.h"
#include "prediction/util/logging.h"
#include "prediction/util/json_util.h"

namespace prediction {

void Response::AddMember(const char* key, const std::string& value) {
  rapidjson::Document::AllocatorType& allocator = res_json_doc_->GetAllocator();
  // res_json_doc_->AddMember(JSON_STRING(key), JSON_STRING(value), allocator);
  res_json_doc_->AddMember(
      // rapidjson::StringRef(key),
      rapidjson::Value().SetString(key, allocator).Move(),
      rapidjson::Value().SetString(value.c_str(), allocator).Move(), allocator);
}

void Response::AddMember(std::string key, const std::string& value) {
  rapidjson::Document::AllocatorType& allocator = res_json_doc_->GetAllocator();
  // res_json_doc_->AddMember(JSON_STRING(key), JSON_STRING(value), allocator);
  res_json_doc_->AddMember(JSON_STRING(key),
      rapidjson::Value().SetString(value.c_str(), allocator).Move(), allocator);
}

std::string Response::json_response() {
  rapidjson::StringBuffer res_buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(res_buffer);
  res_json_doc_->Accept(writer);
  return std::string(res_buffer.GetString());
}

std::string Response::ToString() {
  const std::string response_type_str = "RESPONSE";
  return response_type_str + "  " + json_response();
}

int Response::GetStringParamFromJson(std::string key, std::string* value) {
  if (res_json_doc_->HasMember(key.c_str())
      && (*res_json_doc_)[key.c_str()].IsString()) {
    *value = std::string((*res_json_doc_)[key.c_str()].GetString());
    return 0;
  } else {
    LOG(WARNING) << "Key not found in request, rid:" << rid_ << " key:" << key;
    return -1;
  }
}

// bool Response::IsSuccess() {
//   std::string status;
//   if (GetStringParamFromJson("status", &status) == 0) {
//     if (status == "false") {
//       return false;
//     }
//   }
//   return true;
// }

}  // namespace prediction

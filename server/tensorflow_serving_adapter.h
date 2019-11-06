// CopyRight 2019 360. All rights reserved.
// File   tensorflow_serving_adapter.h
// Date   2019-11-06 17:00:07
// Brief

#ifndef PREDICTION_SERVER_SERVER_TENSORFLOW_SERVING_ADAPTER_H_
#define PREDICTION_SERVER_SERVER_TENSORFLOW_SERVING_ADAPTER_H_

#include <string>
#include <vector>

#include "prediction/third_party/grpc/include/grpcpp/client_context.h"
#include "prediction/third_party/grpc/include/grpc++/security/credentials.h"
#include "prediction/third_party/protobuf/include/google/protobuf/map.h"
#include "prediction/third_party/protobuf/include/google/protobuf/wrappers.pb.h"

#include "prediction/util/spin_mutex.h"
#include "prediction/util/logging.h"

namespace prediction {

enum FeatureValueType {
    FEATURE_VALUE_INT64 = 0,
    FEATURE_VALUE_STRING,
    FEATURE_VALUE_FLOAT,
    FEATURE_VALUE_UNKNOWN
};

// Features info
struct FeatureTuple {
  std::string feature_name;
  // Feature value
  std::string str_feature_value;
  int64_t int64_feature_value;
  float float_feature_value;
    
  FeatureValueType feature_value_type;

  FeatureTuple() {  }

  FeatureTuple(const std::string feature_name, const std::string&& feature_value)
      : feature_name(feature_name),
        str_feature_value(feature_value),
        feature_value_type(FEATURE_VALUE_STRING) {
  }
  FeatureTuple(const std::string feature_name, int64_t feature_value)
      : feature_name(feature_name),
        int64_feature_value(feature_value),
        feature_value_type(FEATURE_VALUE_INT64) {
  }
  FeatureTuple(const std::string feature_name, double feature_value)
      : feature_name(feature_name),
        float_feature_value(feature_value),
        feature_value_type(FEATURE_VALUE_FLOAT) {
  }

  ~FeatureTuple() { }

};

// Meta info for models
struct ModelConfig {
  std::string name;         // model name
  std::string signature;    // model signature
  std::string inputs_key;   // inputs or example
  std::string outputs_key;  // eg scores
  uint64_t version = 0;     // model version
  ModelConfig(std::string n, std::string s, std::string i, std::string o,
              uint64_t v)
    : name(n), signature(s), inputs_key(i), outputs_key(o), version(v) {
  }
};

class TensorflowServingAdapter {
 public:
  TensorflowServingAdapter();
  int Predict(int seq, const std::string& rid, const ModelConfig& model_config,
              const std::string& model_name,
              const std::vector<std::string>& features,
              std::vector<double>* scores);

  static const ModelConfig& model_config_tf() {
    return model_config_tf_;
  }

 private:
  util::SpinMutex enqueue_mutex_;

  static ModelConfig model_config_weight_;
  static ModelConfig model_config_hash_;
  static ModelConfig model_config_tf_;

};  // TensorflowServingAdapter

}  // namespace prediction

#endif  // PREDICTION_SERVER_SERVER_TENSORFLOW_SERVING_ADAPTER_H_

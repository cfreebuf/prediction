// CopyRight 2019 360. All rights reserved.
// File   tensorflow_serving_adapter.cpp
// Date   2019-11-06 17:00:59
// Brief

#include <chrono>
#include <mutex>
#include "prediction/server/tensorflow_serving_adapter.h"
#include "prediction/server/tensorflow_serving_client.h"
#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"
#include "grpc++/security/credentials.h"
#include "google/protobuf/map.h"
#include "google/protobuf/wrappers.pb.h"

#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/example/example.pb.h"
#include "tensorflow/core/example/feature.pb.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/util/command_line_flags.h"

#include "prediction/util/time_utils.h"
#include "prediction/util/error_codes.h"
#include "prediction/util/logging.h"
#include "prediction/common/common_gflags.h"

namespace prediction {

using namespace util;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using tensorflow::serving::PredictRequest;
using tensorflow::serving::PredictResponse;
using tensorflow::serving::PredictionService;
using tensorflow::Example;
using google::protobuf::Map;

using TensorMap = Map<tensorflow::string, tensorflow::TensorProto>;

ModelConfig TensorflowServingAdapter::model_config_tf_(
    "model", "predict_y", "input", "y", 0);

TensorflowServingAdapter::TensorflowServingAdapter() {}

int TensorflowServingAdapter::Predict(
    int seq,
    const std::string& rid,
    const ModelConfig& model_config,
    const std::string& model_name,
    const std::vector<std::string>& features,
    std::vector<double>* scores) {
  LOG(INFO) << "start predict tf rid:" << rid << " seq:" << seq;
  if (features.empty()) {
    LOG(WARNING) << "features is empty rid:" << rid;
    return -1;
  }

  TimeRecorder recorder;
  recorder.StartTimer("total_latency");

  PredictRequest request;

  // Config model
  auto* model_spec = request.mutable_model_spec();
  //model_spec->set_name(model_config.name);
  model_spec->set_name(model_name);
  model_spec->set_signature_name(model_config.signature);
  if (model_config.version > 0) {
    model_spec->mutable_version()->set_value(model_config.version);
  }

  TensorMap& inputs = *request.mutable_inputs();

  tensorflow::TensorProto& tensor_proto = inputs[model_config.inputs_key];
  tensor_proto.set_dtype(tensorflow::DataType::DT_STRING);

  for (auto & r : features) {
    tensor_proto.add_string_val(std::move(r));
  }

  tensor_proto.mutable_tensor_shape()->add_dim()->set_size(features.size());

  PredictResponse response;
#if 0
  if (FLAGS_debug) {
    std::string content ;
    google::protobuf::TextFormat::PrintToString(request, &content);
    LOG(INFO) << "tensorflow Predict request:"<< content << ",rid:" << rid;
  }
#endif

  recorder.StartTimer("predict_latency");
  int predict_status = TensorflowServingClient::Predict(request, response);
  recorder.StopTimer("predict_latency");

#if 0
  if (FLAGS_debug) {
    std::string content ;
    google::protobuf::TextFormat::PrintToString(response, &content);
    LOG(INFO) << "tensorflow Predict response:" << content << ",rid:" << rid; 
  }
#endif

  if (predict_status == 0) {
    TensorMap& outputs = *response.mutable_outputs();
    tensorflow::TensorProto& score_proto = outputs[model_config.outputs_key];
    int value_size = score_proto.float_val_size();

    if (value_size == 0 || value_size != features.size()) {
      LOG(WARNING) << "invalid value_size:" << value_size
        << " sample_size:" << features.size();
      return -1;
    }

    // scores should be resized before
    for (size_t i = 0; i < value_size; i++) {
      (*scores)[i] = score_proto.float_val(i);
    }
  } else {
    LOG(WARNING) << "predict tf error rid:" << rid;
  }

  recorder.StopTimer("total_latency");

  LOG(INFO) << "predict tf rid:" << rid
            << " total_latency:" << recorder.GetElapse("total_latency")
            << " predict_latency:" << recorder.GetElapse("predict_latency");
  return predict_status;
}

}  // namespace prediction

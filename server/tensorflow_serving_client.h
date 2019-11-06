// CopyRight 2019 360. All rights reserved.
// File   tensorflow_serving_client.h
// Date   2019-10-29 13:22:42
// Brief

#ifndef PREDICTION_SERVER_SERVER_TENSORFLOW_CLIENT_H_
#define PREDICTION_SERVER_SERVER_TENSORFLOW_CLIENT_H_

#include "tensorflow_serving/apis/prediction_service.grpc.pb.h"
#include "prediction/util/connection_pool.h"
// #include "prediction/util/time_utils.h"

namespace prediction {

class TensorflowServingClient : public util::Connection {
 public:
  TensorflowServingClient();
  virtual ~TensorflowServingClient() {}
  static int Init();
  virtual int Open();
  virtual void Close();

  static int Predict(const tensorflow::serving::PredictRequest& request,
                     tensorflow::serving::PredictResponse& response);

 private:
  std::unique_ptr<tensorflow::serving::PredictionService::Stub> stub_;
};

class TensorflowServingClientFactory : public util::ConnectionFactory {
 public:
  TensorflowServingClientFactory(const int32_t retry, const int32_t interval_ms)
    : ConnectionFactory(retry, interval_ms) {}
  virtual std::shared_ptr<util::Connection> CreateConnection() {
    return std::make_shared<TensorflowServingClient>();
  }
}; 

}  // namespace prediction
#endif  //  PREDICTION_SERVER_SERVER_TENSORFLOW_CLIENT_H_

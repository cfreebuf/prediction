// CopyRight 2019 360. All rights reserved.
// File   tensorflow_serving_client.cpp
// Date   2019-10-28 14:37:38
// Brief

#include "tensorflow_serving_client.h"
// #include "grpc++/create_channel.h"
#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "prediction/common/common_gflags.h"
#include "prediction/util/logging.h"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

namespace prediction {

using grpc::ClientContext;
using grpc::Status;
using grpc::Channel;
using tensorflow::serving::PredictRequest;
using tensorflow::serving::PredictResponse;
using tensorflow::serving::PredictionService;

using util::TimeRecorder;
using util::Connection;
using util::ConnectionFactory;
using util::ConnectionPoolManager;
using util::ConnectionPool;

TensorflowServingClient::TensorflowServingClient() {
  // if (Init() != 0) {
    // LOG(FATAL) << "Init tensorflow serving client failed";
  // }
}

int TensorflowServingClient::Init() {
  ConnectionPoolManager& manager = ConnectionPoolManager::Instance();
  manager.RegisterConnectionPool(FLAGS_tensorflow_serving_name);
  std::shared_ptr<ConnectionFactory> factory
    = std::make_shared<TensorflowServingClientFactory>(
        FLAGS_tensorflow_serving_conn_retry_times,
        FLAGS_tensorflow_serving_conn_retry_interval_ms);
  int r = manager.GetConnectionPool(FLAGS_tensorflow_serving_name)->Init(factory,
                  FLAGS_tensorflow_serving_min_conn,
                  FLAGS_tensorflow_serving_max_conn,
                  FLAGS_tensorflow_serving_conn_rebalance_second);
  if (r != 0){
    LOG(ERROR) << "TensorflowServingClient init error" ;
  } else {
    LOG(INFO) << "TensorflowServingClient init success,"
              << ", min_conn:" << FLAGS_tensorflow_serving_min_conn
              << ", max_conn:" << FLAGS_tensorflow_serving_max_conn;
  }
  return r;
}

int TensorflowServingClient::Open() {
  // std::string serving_addr;
  grpc::string serving_addr;
  std::vector<std::string> addrs;
  boost::split(addrs, FLAGS_tensorflow_serving_addr, boost::is_any_of(","),
               boost::token_compress_off);
  if (!addrs.empty()) {
    serving_addr = addrs.at(rand() % addrs.size()) ;
  } else {
    LOG(ERROR) << "TensorflowServingClient open failed, addr not set";
    return -1;
  }

  // Create grpc channel
  std::shared_ptr<Channel> channel = grpc::CreateChannel(serving_addr,
      grpc::InsecureChannelCredentials());

  if (channel == nullptr) {
    LOG(WARNING) << "TensorflowServingClient open failed, create channel error";
    return -1;
  }

  // Init stub
  stub_ = PredictionService::NewStub(channel);
  LOG(INFO) << "TensorflowServingClient create successfully, addr:" << serving_addr;
  return 0;
}

void TensorflowServingClient::Close() {}

int TensorflowServingClient::Predict(const PredictRequest& request,
                                     PredictResponse& response) {
  int32_t retry_times = 0;
  int ret = -1;
  Status status;
  ConnectionPoolManager& manager = ConnectionPoolManager::Instance();
  while (retry_times++ < FLAGS_tensorflow_serving_conn_retry_times) {
    std::shared_ptr<ConnectionPool> pool = manager.GetConnectionPool(FLAGS_tensorflow_serving_name);
    if (nullptr == pool) {
      LOG(ERROR) << "Get ConnectionPool failed, name:"<< FLAGS_tensorflow_serving_name << ", retry:" << retry_times ;
      continue ;
    }
    std::shared_ptr<Connection> conn = pool->GetConnection();
    if (conn == nullptr) {
      LOG(ERROR) << FLAGS_tensorflow_serving_name << " Get Connection error";
      continue;
    } else {
      LOG(INFO) << FLAGS_tensorflow_serving_name << " Get Connection successfully";
    }
    std::shared_ptr<TensorflowServingClient> fconn = std::dynamic_pointer_cast<TensorflowServingClient>(conn);

    // Timeout for grpc service 'Predict'
    ClientContext context;
    // std::chrono::system_clock::time_point deadline = 
      // std::chrono::system_clock::now() 
      // + std::chrono::milliseconds(FLAGS_client_connection_timeout);
    // context.set_deadline(deadline);

    LOG(INFO) << "Start Predict from grpc";
    fconn->stub_->Predict(&context, request, &response);
    status = fconn->stub_->Predict(&context, request, &response);
    if (!status.ok()) {
      LOG(ERROR) << FLAGS_tensorflow_serving_name << " Predict error, code:" << status.error_code()
                 << ", msg:" << status.error_message() << ", retry:" << retry_times;
      pool->Release(fconn, true);
      continue;
    } else {
      ret = 0;
      LOG(INFO) << FLAGS_tensorflow_serving_name << " Predict successfully";
    }
    pool->Release(fconn, false);
    break;
  }
  return ret;
}

}

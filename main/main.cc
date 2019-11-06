// CopyRight 2019 360. All rights reserved.
// File   main.cc
// Date   2019-11-01 19:08:36
// Brief

#include <chrono>
#include <thread>
#include <signal.h>

#include "prediction/third_party/glog/include/glog/logging.h"
#include "prediction/third_party/gflags/include/gflags/gflags.h"
#include "prediction/util/monitor.h"
#include "prediction/util/logging.h"
#include "prediction/util/connection_pool.h"
#include "prediction/util/reloader/data_manager.h"
#include "prediction/server/prediction_manager.h"
#include "prediction/chain/request_processor.h"
#include "prediction/common/common_gflags.h"
#include "prediction/server/prediction_server.h"
#include "prediction/server/tensorflow_serving_client.h"
#include "prediction/server/tensorflow_serving_adapter.h"

// using namespace tensorflow;
using namespace google;
using namespace prediction;
using namespace prediction::util;

// Reload gflags
std::unique_ptr<gflags::FlagSaver> current_flags(new gflags::FlagSaver());
void StartConfigReloadThread() {
    std::thread([]() {
        while (1) {
            current_flags.reset();
            current_flags.reset(new gflags::FlagSaver());
            gflags::ReparseCommandLineNonHelpFlags();
            // Reload conf/gflags.conf
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }).detach();
}

int main(int argc, char** argv) {
  // Init gflags and glog
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  gflags::SetCommandLineOption("flagfile", "./conf/gflags.conf");
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();
  // google::FlushLogFiles(google::WARNING);
  google::FlushLogFiles(google::INFO);
  // FLAGS_log_dir = "./logs";
  FLAGS_logbufsecs = 0;
  FLAGS_alsologtostderr = true;
  StartConfigReloadThread();
  LOG(INFO) << "Start prediction server";

  // Init monitor
  monitor::Monitor::GetInstance().Init(FLAGS_monitor_data, FLAGS_monitor_status,
                                       FLAGS_monitor_interval);
  monitor::Monitor::GetInstance().Start();
  LOG(INFO) << "Monitor inited successfully";

  if (TensorflowServingClient::Init() != 0) {
    return -1;
  }

  RequestProcessor request_processor;
  if (request_processor.ReloadExpConf(FLAGS_exp_conf_path) != 0) {
    LOG(FATAL) << "load exp conf error";
    return -1;
  }

  // Init data manager
  // reloader::DataManager& data_manager = reloader::DataManager::Instance();
  // data_manager.Init();

  // 加载prediction配置文件
  PredictionManager& prediction_manager = PredictionManager::GetInstance();
  prediction_manager.Start();

  // Start prediction server
  PredictionServer server;
  server.Start();

  ConnectionPoolManager::Instance().Destory();

  LOG(INFO) << "PredictionServer exited";

  return 0;
}

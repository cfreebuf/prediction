// File   rank_request_processor.cpp
// Author lidongming
// Date   2018-09-25 21:30:34
// Brief

#include <thread>
#include <chrono>
#include "chain/request_processor.h"
#include "common/common_gflags.h"
#include "util/logging.h"

namespace prediction {

RequestProcessor::RequestProcessor() {
  exp_handler_.Init();
}

RequestProcessor::~RequestProcessor() { }

int RequestProcessor::Process(Request* request) {
  return exp_handler_.DoExp(*request);
}

int RequestProcessor::StartReloadThread() {
  std::thread([this] {
    while(true) {
    std::this_thread::sleep_for(std::chrono::seconds(FLAGS_reload_exp_interval));
      ReloadExpConf(FLAGS_exp_conf_path);
      // LOG(INFO) << "Reload exp config successfully";
    }
  }).detach();
  return 0;
}

int RequestProcessor::ReloadExpConf(const std::string& conf_path) {
  return exp_handler_.Reload(conf_path);
}

}

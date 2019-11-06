// CopyRight 2019 360. All rights reserved.
// File   prediction_manager.cpp
// Date   2019-10-29 11:05:36
// Brief

#include <thread>
#include <memory>
#include "prediction/server/prediction_manager.h"
#include "prediction/third_party/libconfig/include/libconfig.h++"
#include "prediction/common/common_gflags.h"
#include "util/logging.h"

namespace prediction {

using namespace util::reloader;

PredictionManager::PredictionManager() {
  reloader_meta_.type = STATE_CHECKER_FILE;
  reloader_meta_.file_name = FLAGS_prediction_conf_file;
  reloader_meta_.interval = FLAGS_reload_prediction_interval;

  predictions_reloader_ = std::make_shared<
    ReloaderEntry<PredictionReloader, ReloaderMeta>>(
      reloader_meta_);
  predictions_reloader_->Load(reloader_meta_);
}

PredictionManager::~PredictionManager() { }

void PredictionManager::Start() {
  running_ = true;

  std::thread(&PredictionManager::ReloadPredictionConfThread, this).detach();
  LOG(INFO) << "prediction manager started";
}

void PredictionManager::ReloadPredictionConfThread() {
  while (running_) {
    sleep(FLAGS_reload_prediction_interval);
    predictions_reloader_->Reload(reloader_meta_);
    // LOG(INFO) << "prediction conf reloaded";
  }
}

int PredictionManager::Reload() {
  return predictions_reloader_->Reload(reloader_meta_);
}

std::shared_ptr<Prediction> PredictionManager::GetPrediction(
    const std::string& prediction_id) {
  if (!predictions_reloader_ || !predictions_reloader_->GetContent()) {
    return nullptr;
  }
  return predictions_reloader_->GetContent()->GetPrediction(prediction_id);
}

int PredictionReloader::Load() {
  LOG(INFO) << "load prediction conf:" << reloader_meta_.file_name;

  libconfig::Config conf;
  conf.readFile(reloader_meta_.file_name.c_str());
  const libconfig::Setting& predictions_conf = conf.lookup("predictions");
  int count = predictions_conf.getLength();

  for (int i = 0; i < count; ++i) {
    std::string model_id;
    if (!predictions_conf[i].lookupValue(std::string("model_id"), model_id)
        || model_id.empty()) {
      LOG(WARNING) << "no model_id found:" << FLAGS_prediction_conf_file;
      continue;
    }

    std::string prediction_id;
    if (!predictions_conf[i].lookupValue(std::string("prediction_id"), prediction_id)
        || prediction_id.empty()) { 
      prediction_id = model_id;
    }

    if (_predictions.find(prediction_id) != _predictions.end()) {
      LOG(WARNING) << "repeated prediction_id:" << prediction_id;
      return -1;
    }

    std::shared_ptr<Prediction> prediction = std::make_shared<Prediction>();
    prediction->prediction_id = prediction_id;
    prediction->model_id = model_id;

    LOG(INFO) << "load prediction prediction_id:" << prediction_id
              << " model_id:" << model_id;

    _predictions[prediction_id] = prediction;
  }
  return 0;
}

std::shared_ptr<Prediction> PredictionReloader::GetPrediction(
    const std::string& prediction_id) {
  auto it = _predictions.find(prediction_id);
  if (it != _predictions.end()) {
    return it->second;
  }
  return NULL;
}

}  // namespace prediction

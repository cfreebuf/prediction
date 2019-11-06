// CopyRight 2019 360. All rights reserved.
// File   prediction_manager.h
// Date   2019-10-29 11:05:04
// Brief

#ifndef PREDICTION_SERVER_MODEL_PREDICTION_MANAGER_H_
#define PREDICTION_SERVER_MODEL_PREDICTION_MANAGER_H_

#include <pthread.h>
#include <string>
#include <memory>
#include <map>
#include <unordered_map>

#include "prediction/server/prediction.h"
#include "prediction/util/reloader/reloader.h"

namespace prediction {
using namespace util::reloader;

class PredictionReloader {
 public:
  PredictionReloader(ReloaderMeta meta) : reloader_meta_(meta) {
  }
  int Load();
  std::shared_ptr<Prediction> GetPrediction(const std::string& prediction_id);

 private:
  std::map<std::string, std::shared_ptr<Prediction>> _predictions;
  ReloaderMeta reloader_meta_;
};

class PredictionManager {
 public:
  PredictionManager();
  ~PredictionManager();

  static PredictionManager& GetInstance() {
      static PredictionManager instance;
      return instance;
  }

  void Start();
  void Stop() { running_ = false; }

  int Reload();

  std::shared_ptr<Prediction> GetPrediction(const std::string& prediction_id);

  void PredictionThread();
  void ReloadPredictionConfThread();

 private:
  volatile bool running_;

  ReloaderMeta reloader_meta_;
  std::shared_ptr<ReloaderEntry<PredictionReloader, ReloaderMeta>>
      predictions_reloader_;

};  // PredictionManager

}  // namespace prediction

#endif  // PREDICTION_SERVER_MODEL_PREDICTION_MANAGER_H_

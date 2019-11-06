// CopyRight 2019 360. All rights reserved.
// File   data_manager.h
// Date   2019-10-29 11:00:54
// Brief

#ifndef RELOADER_RELOADER_DATA_MANAGER_H_
#define RELOADER_RELOADER_DATA_MANAGER_H_

#include <chrono>
#include <thread>
#include "prediction/util/reloader/reloader.h"
#include "prediction/common/common_gflags.h"

namespace prediction {
namespace util {
namespace reloader {
 
template<typename Entry, class... Args>
class ReloaderDetachThread {
 public:
   ReloaderDetachThread(ReloaderMeta reloader_meta)
       : reloader_meta_(reloader_meta) {
     entry_ = std::make_shared<Entry>(reloader_meta); 
   }

   void Start() {
     entry_->Load(reloader_meta_);
     LOG(INFO) << "load finished type:" << reloader_meta_.type
               << " file:" << reloader_meta_.file_name
               << " redis_key:" << reloader_meta_.redis_key;


     std::thread([this]() {
       while (true) {
         entry_->Reload(reloader_meta_);
         std::this_thread::sleep_for(
             std::chrono::seconds(reloader_meta_.interval));
       }
     }).detach();
  }

   std::shared_ptr<Entry> GetEntry() {
     return entry_;
   }

 private:
   std::shared_ptr<Entry> entry_;
   ReloaderMeta reloader_meta_;
};

#if 0
class DataManager {
 public:
  static DataManager& Instance() {
    static DataManager instance;
    return instance;
  }

  using FeatureIdReloaderEntry = ReloaderEntry<
                             VectorReloader<int, std::string,
                               CommonParser<int, std::string>>, ReloaderMeta>; 

  using FeatureIdReloadThread = ReloaderDetachThread<FeatureIdReloaderEntry,
          ReloaderMeta>;

  void Init() {
    // Reload meta for redis
    // ReloaderMeta reloader_meta;
    // reloader_meta.type = STATE_CHECKER_REDIS;
    // reloader_meta.redis_host = FLAGS_redis_host;
    // reloader_meta.redis_key = FLAGS_feature_id_redis_key;
    // reloader_meta.redis_tm_key = FLAGS_feature_id_redis_tm_key;

    // Reload meta for local file
    ReloaderMeta reloader_meta_tf;
    reloader_meta_tf.type = STATE_CHECKER_FILE;
    reloader_meta_tf.value_seprator = '\t';
    reloader_meta_tf.file_name = "./conf/tf.conf";
    reloader_meta_tf.interval = FLAGS_reloader_interval;
    feature_id_tf_reloader_thread_ = std::make_shared<FeatureIdReloadThread>(reloader_meta_tf);
    feature_id_tf_reloader_thread_->Start();

    check();
  }

  std::shared_ptr<std::vector<int>> GetFeatureIdTFVectorDB() {
    auto entry = feature_id_tf_reloader_thread_->GetEntry();
    if (entry == nullptr) {
      LOG(ERROR) << "entry is null";
      return nullptr;
    }
    auto reloader = entry->GetContent();
    if (reloader == NULL) {
      LOG(ERROR) << "reloader is null";
      return nullptr;
    }
    auto db = reloader->GetDB();
    if (db == nullptr) {
      LOG(ERROR) << "db is null";
      return nullptr;
    }
    return db;
  }

  bool check() {
    // Check tf_ids
    std::shared_ptr<std::vector<int>> tf_ids = GetFeatureIdTFVectorDB();

    if (tf_ids == nullptr || tf_ids->empty()) {
      LOG(ERROR) << "invalid tf_ids";
      return false;
    }

    for (int v : *tf_ids) {
      LOG(INFO) << "[tf_ids] " << v;
    }
    LOG(INFO) << "tf_ids size:" << tf_ids->size();
    return true;
  }

 private:
  std::shared_ptr<FeatureIdReloadThread> feature_id_tf_reloader_thread_;
};  // DataManager
#endif

}  // namespace reloader
}  // namespace util
}  // namespace prediction
#endif  // RELOADER_RELOADER_DATA_MANAGER_H_

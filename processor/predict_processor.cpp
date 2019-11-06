// CopyRight 2019 360. All rights reserved.
// File   predict_processor.cpp
// Date   2019-10-29 12:03:31
// Brief

#include "prediction/processor/predict_processor.h"
#include "prediction/util/util.h"
#include "prediction/util/monitor.h"
#include "prediction/util/logging.h"
#include "prediction/util/time_utils.h"
#include "prediction/util/threadpool/env.h"
#include "prediction/util/threadpool/env_time.h"
#include "prediction/util/threadpool/threadpool.h"
#include "prediction/util/reloader/data_manager.h"
#include "prediction/server/prediction.h"
#include "prediction/server/prediction_manager.h"
#include "prediction/server/tensorflow_serving_adapter.h"
// #include "prediction/client/feature_engine_client.h"
#include "prediction/common/common_gflags.h"
#include "prediction/common/errcode.h"

namespace prediction {

using namespace util;

struct ScoreDocIndex {
  double score;
  int index;
  ScoreDocIndex(double s, int i) : score(s), index(i) {}
};
static bool compare_score_doc(const ScoreDocIndex& a, const ScoreDocIndex& b) {
  return a.score > b.score;
}

// thread::ThreadPool predict_threadpool(Env::Default(), "predict_threadpool",
                              // std::thread::hardware_concurrency());

PredictProcessor::PredictProcessor() { }
PredictProcessor::~PredictProcessor() { }

int PredictProcessor::Process(Context& context) {
    Predict(context);

    CallNextProcessor(context);

    return 0;
}

int PredictProcessor::Predict(Context& context) {
  Request* request = context.request();
  int ret = -1;
  if (FLAGS_enable_predict_parallel) {
    ret = PredictTFParallel(context);
  } else {
    ret = PredictTF(context);
  }
  return ret;
}

int PredictProcessor::PredictTF(Context& context) {
#if 0
  TimeRecorder timer;
  const PredictRequest* request = context.request()->request();
  int docs_size = request->docs.size();
  context.InitScores(docs_size, 0.0);

  LOG(INFO) << "predict tf rid:" << request->rid;

  // 获取TF对应的特征集合
  std::shared_ptr<std::vector<int>> feature_ids
    = reloader::DataManager::Instance().GetFeatureIdTFVectorDB();

  LOG(INFO) << "predict tf start get features rid:" << request->rid;

  // 请求特征服务以获取TF特征
  feature_thrift::FeatureResponse feature_res;
  using FeatureType = feature_thrift::FeatureType::type;

  TimeRecorder recorder;
  recorder.StartTimer("client_feature");
  const std::string& prediction_id = context.request()->Get("prediction_id");
  FeatureEngineClient::GetFeatures(prediction_id, *feature_ids,
                                   FeatureType::TF_FEATURE, *request, 0,
                                   docs_size, false, NULL, &feature_res,
                                   *context.request());
  recorder.StopTimer("client_feature");

  const std::vector<std::string>& features = feature_res.serialized_features;
  LOG(INFO) << "predict tf finish get features rid:" << request->rid
            << " features_size:" << features.size()
            << " feature_ids_size:" << feature_ids->size()
            << " docs_size:" << docs_size
            << " latency:" << recorder.GetElapse("client_feature");

  LOG(INFO) << "start tf predict rid:" << request->rid;

  // 请求Tensorflow Serving并传入特征进行预估
  const ModelConfig& model_config = TensorflowServingAdapter::model_config_tf();
  TensorflowServingAdapter adapter;
  Status status = adapter.Predict(0, request->rid, model_config, prediction_id, features,
                                  &context.scores());

  LOG(INFO) << "finish tf predict rid:" << request->rid << " status:" << status;
  if (status.ok()) {
    return 0;
  } else {
    context.set_err_code(PREDICT_TF_TIMEOUT);
    return -1;
  }
#endif
}

int PredictProcessor::PredictTFParallel(Context& context) {
#if 0
  TimeRecorder timer;
  timer.StartTimer("predicttf_all");
  const PredictRequest* request = context.request()->request();
  int docs_size = request->docs.size();
  context.InitScores(docs_size, 0.0);

  LOG(INFO) << "predict tf parallel rid:" << request->rid
            << " docs_size:" << docs_size;

  // 获取TF对应的特征集合
  std::shared_ptr<std::vector<int>> feature_ids
    = reloader::DataManager::Instance().GetFeatureIdTFVectorDB();

  // 请求特征服务以获取TF特征
  feature_thrift::FeatureResponse feature_res;
  using FeatureType = feature_thrift::FeatureType::type;

  int task_count = docs_size / FLAGS_tf_predict_parallel_task_docs_count;
  if (docs_size % FLAGS_tf_predict_parallel_task_docs_count > 0) {
    task_count += 1;
  }

  TaskTracker task_tracker(task_count);

  std::vector<std::vector<double>> scores;
  scores.resize(task_count);

  for (int i = 0; i < task_count; i++) {
    int start = i * FLAGS_tf_predict_parallel_task_docs_count;
    int end = (i + 1) * FLAGS_tf_predict_parallel_task_docs_count;
    if (end > docs_size) {
      end = docs_size;
    }

    if (end > start) {
      scores[i].resize(end - start);
    }

    LOG(INFO) << "prepare predict tf parallel impl rid:" << request->rid
              << " seq:" << i << " start:" << start << " end:" << end;

    PredictTask task;
    task.seq = i;
    task.start = start;
    task.end = end;
    task.prediction_id = context.request()->Get("prediction_id");
    task.feature_ids = feature_ids.get();
    task.feature_type = FeatureType::TF_FEATURE;
    task.predict_request = request;
    task.request = context.request();
    task.scores = &scores[i];
    task.task_tracker = &task_tracker;

    predict_threadpool.Schedule([=]() {
      PredictTFParallelImpl(task);
      task.task_tracker->done();
    });
  }

  std::unique_lock<std::mutex> l(task_tracker.done_lock);
  if (!task_tracker.done_flag) {
    task_tracker.done_cv.wait(l);
  }

  std::vector<double>& target_scores = context.scores();
  int k = 0;
  for (int i = 0; i < task_count; i++) {
    for (int j = 0; j < scores[i].size(); j++) {
      target_scores[k++] = scores[i][j];
    }
  }
  timer.StopTimer("predicttf_all");

  LOG(INFO) << "finished predict tf parallel rid:" << request->rid
            << " docs_size:" << docs_size
            << " latency:" << timer.GetElapse("predicttf_all");
#endif
  return 0;
}

int PredictProcessor::PredictTFParallelImpl(PredictTask task) {
#if 0
  TimeRecorder timer;
  timer.StartTimer("predict_tf_task_features");
  feature_thrift::FeatureResponse features;
  FeatureEngineClient::GetFeatures(task.prediction_id, *task.feature_ids,
                                   task.feature_type, *task.predict_request,
                                   task.start, task.end, false, NULL,
                                   &features, *task.request);
  timer.StopTimer("predict_tf_task_features");

  timer.StartTimer("predict_tf_task_predict");
  // 请求Tensorflow Serving并传入特征进行预估
  const ModelConfig& model_config = TensorflowServingAdapter::model_config_tf();
  TensorflowServingAdapter adapter;
  Status status = adapter.Predict(task.seq, task.request->rid(), model_config,
                                  task.prediction_id, features.serialized_features,
                                  task.scores);

  timer.StopTimer("predict_tf_task_predict");

  LOG(INFO) << "predict tf parallel impl rid:" << task.request->rid()
            << " docs_size:" << task.end - task.start 
            << " features_latency:"
            << timer.GetElapse("predict_tf_task_features")
            << " predict_latency:"
            << timer.GetElapse("predict_tf_task_predict")
            << " status:" << status.ok();
#endif
  return 0;
}

}  // namespace prediction

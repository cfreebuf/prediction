// CopyRight 2019 360. All rights reserved.
// File   task.h
// Date   2019-10-29 12:07:01
// Brief

#ifndef PREDICTION_SERVER_MODEL_TASK_H_
#define PREDICTION_SERVER_MODEL_TASK_H_

#include <mutex>
#include <atomic>
#include <vector>
#include <string>
#include <condition_variable>

namespace prediction {

struct TaskTracker {
  TaskTracker(int task_count) {
    count.store(task_count);
  }
  std::atomic_int_fast32_t count;
  std::mutex done_lock;
  std::condition_variable done_cv;
  bool done_flag = false;

  void done() {
    if (count.fetch_sub(1) == 1) {
      // mutex_lock l(done_lock);
      std::unique_lock<std::mutex> l(done_lock);
      done_flag = true;
      done_cv.notify_all();
    }
  }
};

class Request;
class Model;
struct PredictTask {
  int seq = 0;
  int start = 0;
  int end = 0;
  std::string prediction_id;
  std::vector<int>* feature_ids = NULL;
  Request* request = NULL;
  std::vector<double>* scores = NULL;
  TaskTracker* task_tracker = NULL;
};

struct ScoreTask {
  int seq = 0;
  int start = 0;
  int end = 0;
  Request* request = NULL;
  std::vector<std::vector<int64_t>>* hash_features = NULL;
  std::vector<double>* scores = NULL;
  TaskTracker* task_tracker = NULL;
};

}

#endif

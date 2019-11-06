// CopyRight 2019 360. All rights reserved.
// File   concurrent_queue.h
// Date   2019-10-28 15:03:22
// Brief

#ifndef PREDICTION_SERVER_UTIL_CONCURRENT_QUEUE_H_
#define PREDICTION_SERVER_UTIL_CONCURRENT_QUEUE_H_

#include <queue>
#include <mutex>
#include <condition_variable>

namespace prediction {
namespace util {

template<class Data>
class ConcurrentQueue {
 public:
  ConcurrentQueue() {}
  virtual ~ConcurrentQueue() {}

  virtual void Push(const Data& data) {
    std::unique_lock<std::mutex> lock(mutex_);
    queue_.emplace(data);
    condition_variable_.notify_one();
  }

  virtual void Push(const std::vector<Data>& data) {
    std::unique_lock<std::mutex> lock(mutex_);
    for (const Data& d : data) {
      queue_.emplace(d);
    }
    condition_variable_.notify_one();
  }

  virtual void Pop(Data& data) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
      condition_variable_.wait(lock);
    }
    data = queue_.front();
    queue_.pop();
  }

  virtual bool PopWait(Data& data, int s) {
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty()) {
        if (condition_variable_.wait_for(lock, std::chrono::seconds(s))
                == std::cv_status::timeout) {
            return false;
        }
    }
    data = queue_.front();
    queue_.pop();
    return true;
  }

  bool TryPop(Data& data) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (queue_.empty()) {
      return false;
    }
    data = queue_.front();
    queue_.pop();
    return true;
  }

  Data const& Front() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.front();
  }

  bool Empty() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.empty();
  }

  int Size() const {
    std::unique_lock<std::mutex> lock(mutex_);
    return queue_.size();
  }

 protected:
  std::queue<Data> queue_;
  mutable std::mutex mutex_;
  std::condition_variable condition_variable_;

 private:
  // DISALLOW_COPY_AND_ASSIGN(ConcurrentQueue);
};  // ConcurrentQueue

}  // namespace util
}  // namespace prediction

#endif  // PREDICTION_SERVER_UTIL_CONCURRENT_QUEUE_H_

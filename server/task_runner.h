// File   task_runner.h
// Author lidongming
// Date   2018-09-11 21:04:18
// Brief

#include <vector>
#include <atomic>
#include <string.h>
#include "prediction/deps/commonlib/include/concurrent_queue.h"
#include "prediction/server/task.h"
#include "prediction/util/logging.h"

namespace prediction {

class TaskRunner {
 public:
   explicit TaskRunner(int thread_count) {
#if 0
     tasks_.resize(10000000);
     tasks_write_index_.store(0);
     tasks_read_index_.store(0);
#endif

     for (int i = 0; i < thread_count; i++) {
       std::thread t(&TaskRunner::Run, this);
       t.detach();
     }
     LOG(INFO) << "start task runner threads:" << thread_count;
   }
   void Run() {
     while(true) {
#if 1
       Task task;
       task_queue_.Pop(task);
       LOG(INFO) << "pop task rid:" << task.rid << " task:" << task.task;
       task.callback();
       LOG(INFO) << "finish task rid:" << task.rid << " task:" << task.task;
       task.task_tracker->done();
#endif
#if 0
       Task& task = tasks_[tasks_read_index_.fetch_add(1)];

       if (!task.valid) {
         tasks_read_index_.fetch_sub(1);
         usleep(500);
       } else {
         task.valid = false;
         LOG(INFO) << "pop task rid:" << task.rid << " task:" << task.task;
         task.callback();
         task.task_tracker->done();
         LOG(INFO) << "finish task rid:" << task.rid << " task:" << task.task;
       }
#endif
     }
   }

   void AddTask(Task& task) {
     task_queue_.Push(task);
#if 0
     tasks_[tasks_write_index_.fetch_add(1)] = task;
#endif
   }

   // void AddTasks(std::vector<Task>& tasks) {
     // task_queue_.PushBatch(tasks);
   // }

 private:
   commonlib::ConcurrentQueue<Task> task_queue_;
#if 0
   std::vector<Task> tasks_;
   std::atomic<int> tasks_write_index_;
   std::atomic<int> tasks_read_index_;
#endif

};  // TaskRunner

}  // namespace prediction

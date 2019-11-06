// CopyRight 2019 360. All rights reserved.
// File   connection_pool.h
// Date   2019-10-28 16:29:07
// Brief

#ifndef PREDICTION_SERVER_UTIL_CONNECTION_POOL_H_
#define PREDICTION_SERVER_UTIL_CONNECTION_POOL_H_

// #include <time.h>
// #include <sys/time.h>
// #include <unistd.h>
#include <assert.h>
#include <map>
#include <queue> 
#include <vector>
#include <mutex>
#include <memory>
#include <string>
#include <chrono>
#include <thread>
// #include <sstream>
#include "util/time_utils.h"

namespace prediction { 
namespace util { 

class Connection {
 public:
  Connection() {}
  virtual ~Connection() {}
  virtual int Open() = 0 ;
  virtual void Close() = 0 ;
  // friend class ConnectPool ;
  // friend class ConnectFactory ;

  int OpenImp() {
      this->ts_ = TimeUtils::GetCurrentTimeRound();
      return this->Open();
  }

  void set_index(int32_t index) { index_ = index; }

  int32_t index() const { return index_; }
  int64_t ts() const{ return ts_; }
 
 private:
  int32_t index_;
  int64_t ts_ ;
};

class ConnectionFactory {
 public:
  ConnectionFactory(const int retry_times = 0, const int retry_interval_ms = 0)
    : retry_times_(retry_times), retry_interval_ms_(retry_interval_ms) {}
  virtual ~ConnectionFactory() {}
  virtual std::shared_ptr<Connection> CreateConnection() = 0 ;

  // friend class ConnectPool ;
  std::shared_ptr<Connection> NewConnection() {
    std::shared_ptr<Connection> conn = CreateConnection() ;
    if (nullptr != conn) {
      if (conn->Open() == 0) {
        return conn;
      }
    }
    return nullptr;
  }

  int OpenConnection(std::shared_ptr<Connection>& conn) {
      int32_t retry = 0 ;
      int reply;
      do {
          reply = conn->OpenImp() ;
          if (0 == reply) {
              return reply;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(retry_interval_ms_));
      } while (++retry < retry_times_) ;
      return reply;
  }

 private:
  int32_t retry_times_ ;
  int32_t retry_interval_ms_ ;
};

class ConnectionPool {
 public:
  ConnectionPool() : size_(0), max_conn_(0) {}
  virtual ~ConnectionPool() {} 
  int Init(std::shared_ptr<ConnectionFactory> factory, const int32_t min_conn,
           const int32_t max_conn, const int32_t conn_rebalance_time_second) {
    factory_ = factory;
    max_conn_ = max_conn;
    conn_rebalance_second_ = conn_rebalance_time_second;
    pool_.resize(max_conn);

    for (int32_t i = 0; i < min_conn; ++i) {
      std::shared_ptr<Connection> conn = factory_->NewConnection();
      if (conn == nullptr) {
        return -1;
      }
      conn->set_index(i);
      pool_[i] = conn ;
      idle_.push(i);
      ++size_;
    }
    return 0;
  }

  void Destory() {
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto it = pool_.begin(); it != pool_.end(); ++it) {
      (*it)->Close();
    }
  }

  std::shared_ptr<Connection> GetConnection() {
    int32_t pos = -1 ;
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (!idle_.empty()) {
        pos = idle_.front();
        idle_.pop();
        return pool_.at(pos) ;
      }
    }

    if (size_ < max_conn_) {
      std::shared_ptr<Connection> conn = factory_->NewConnection() ;
      if (conn == nullptr) {
        return nullptr;
      }

      std::unique_lock<std::mutex> lock(mutex_);
      if (size_ < max_conn_) {
        pos = size_++;
        conn->set_index(pos);
        pool_[pos] = conn;
        return conn;
      }
    }
    return nullptr;
  }

  int Release(std::shared_ptr<Connection> conn, bool reopen = false) {
    if (nullptr == conn) {
      return -1;
    }
    int reply = -1;
    int32_t pos = conn->index();
    if (reopen
        || ((TimeUtils::GetCurrentTimeRound() - conn->ts())
              > conn_rebalance_second_)) {
      pool_[pos]->Close();
      reply = factory_->OpenConnection(pool_[pos]);
    }
    {
      std::unique_lock<std::mutex> lock(mutex_);
      if (reply == 0) {
        idle_.push(pos);
      }
    }
    return reply ;
  }

 private:
  int32_t size_;
  int32_t max_conn_;
  int32_t conn_rebalance_second_;

  mutable std::mutex mutex_;
  std::queue<int32_t> idle_;
  std::vector<std::shared_ptr<Connection>> pool_;
  std::shared_ptr<ConnectionFactory> factory_;
};

class ConnectionPoolManager {
 public:
  static ConnectionPoolManager& Instance() {
    static ConnectionPoolManager instance;
    return instance;
  }

  void RegisterConnectionPool(const std::string& name){
    std::unique_lock<std::mutex> lock(mutex_);
    if (manager_.find(name) == manager_.end()) {
      std::shared_ptr<ConnectionPool> p = std::make_shared<ConnectionPool>();
      manager_.insert(std::pair<std::string, std::shared_ptr<ConnectionPool>>(name, p));
    }
  }

  std::shared_ptr<ConnectionPool>& GetConnectionPool(const std::string& name) {
    auto iter = manager_.find(name);
    assert(iter != manager_.end() && iter->second != nullptr);
    return iter->second;
  }

  void Destory() {
    std::unique_lock<std::mutex> lock(mutex_);
    for (auto iter = manager_.begin() ; iter != manager_.end() ; ++iter) {
      iter->second->Destory();
    }
    manager_.clear();
  }
  virtual ~ConnectionPoolManager() {}

 private:
  ConnectionPoolManager() {}

 private:
  std::map<std::string, std::shared_ptr<ConnectionPool>> manager_;
  mutable std::mutex mutex_;
};

}  // namespace util
}  // namespace prediction

#endif  // PREDICTION_SERVER_UTIL_CONNECTION_POOL_H_

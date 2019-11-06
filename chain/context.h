// File   context.h
// Author lidongming
// Date   2018-09-25 22:13:33
// Brief

#ifndef PREDICTION_SERVER_CHAIN_CONTEXT_H_
#define PREDICTION_SERVER_CHAIN_CONTEXT_H_

#include "server/request.h"
#include "server/result.h"

namespace prediction {

class Execution;

class Context {
 public:
  Context(Request* request, Result* result, Execution* exec);
  virtual ~Context();

  Request* request() { return request_; }
  Result* result() { return result_; }
  Execution* execution() { return execution_; }

  std::vector<double>& scores() {
    return scores_;
  }

  void InitScores(int size, double default_value) {
    scores_.resize(size, default_value);
  }

  void UpdateScore(int seq, double value) {
    scores_[seq] = value;
  }

  int err_code() {
    return err_code_;
  }

  void set_err_code(int err_code) {
    err_code_ = err_code;
  }

 private:
  Request* request_;
  Result* result_;
  Execution* execution_;
  std::vector<double> scores_;
  int sample_count_;

  bool generate_textual_features_;
  std::string docid;
  int err_code_;
};  // Context

}  // namespace prediction

#endif  // PREDICTION_SERVER_CHAIN_CONTEXT_H_

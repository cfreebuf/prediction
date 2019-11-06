// CopyRight 2019 360. All rights reserved.
// File   prediction_stats.h
// Date   2019-10-31 10:33:16
// Brief

#ifndef PREDICTION_SERVER_PREDICTION_STATS_H_
#define PREDICTION_SERVER_PREDICTION_STATS_H_

#include <stdint.h>

namespace prediction {

class PredictionStats {
 public:
  virtual ~PredictionStats() {}
  virtual void recordRequest() { ++req_count_; }
  virtual uint64_t getRequestCount() { return req_count_; }

 private:
  uint64_t req_count_{0};
};

}  // namespace prediction
#endif  // PREDICTION_SERVER_PREDICTION_STATS_H_

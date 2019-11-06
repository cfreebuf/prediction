// CopyRight 2019 360. All rights reserved.
// File   predict_processor.h
// Date   2019-11-04 19:04:34
// Brief

#ifndef PREDICTION_SERVER_PROCESSORS_PREDICT_PROCESSOR_H_
#define PREDICTION_SERVER_PROCESSORS_PREDICT_PROCESSOR_H_

#include <string>

#include "processor/processor.h"
#include "chain/context.h"
#include "common/common_define.h"
#include "server/tensorflow_serving_adapter.h"
#include "prediction/server/task.h"
#include "prediction/third_party/rapidjson/rapidjson.h"
#include "prediction/third_party/rapidjson/document.h"
#include "prediction/third_party/rapidjson/stringbuffer.h"
#include "prediction/third_party/rapidjson/writer.h"

namespace prediction {

class PredictProcessor : public Processor {
 public:
  PredictProcessor();
  virtual ~PredictProcessor();

  const std::string name() const { return "predict_processor"; }

  int Process(Context& context);
  int Predict(Context& context);

  int PredictTF(Context& context);
  int PredictNormalParallelImpl(PredictTask task);

  int PredictTFParallel(Context& context);
  int PredictTFParallelImpl(PredictTask task);
};  // PredictProcessor

}  // namespace prediction
#endif  // PREDICTION_SERVER_PROCESSORS_PREDICT_PROCESSOR_H_

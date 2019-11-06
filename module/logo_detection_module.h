// CopyRight 2019 360. All rights reserved.
// File   logo_detection_module.h
// Date   2019-11-05 10:37:48
// Brief

#ifndef PREDICTION_MODULE_LOGO_DETECTION_MODULE_H_
#define PREDICTION_MODULE_LOGO_DETECTION_MODULE_H_

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "prediction/module/prediction_module.h"

namespace prediction {

class LogoDetectionModule : public PredictionModule {
 public:
  LogoDetectionModule(std::string graph_path);
  ~LogoDetectionModule() {}

  util::Status Predict(const Request& request, Response* response) override;

  tensorflow::Status ReadEntireFile(tensorflow::Env* env,
    const std::string& filename, tensorflow::Tensor* output);

  tensorflow::Status ReadTensorFromImageFile(const std::string& file_name,
    const int image_height, const int image_width, const float input_mean,
    const float input_std, std::vector<tensorflow::Tensor>* out_tensors);

 private:
};

}  // namespace prediction
#endif  // PREDICTION_MODULE_LOGO_DETECTION_MODULE_H_

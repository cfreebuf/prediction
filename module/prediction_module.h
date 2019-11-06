// CopyRight 2019 360. All rights reserved.
// File   prediction_module.h
// Date   2019-11-04 11:35:32
// Brief

#ifndef PREDICTION_MODULE_PREDICTION_MODULE_H_
#define PREDICTION_MODULE_PREDICTION_MODULE_H_

#include <memory>
#include <string>
// #include "tensorflow/cc/ops/const_op.h"
// #include "tensorflow/cc/ops/image_ops.h"
// #include "tensorflow/cc/ops/standard_ops.h"
// #include "tensorflow/core/framework/graph.pb.h"
// #include "tensorflow/core/framework/tensor.h"
// #include "tensorflow/core/graph/default_device.h"
// #include "tensorflow/core/graph/graph_def_builder.h"
// #include "tensorflow/core/lib/core/errors.h"
// #include "tensorflow/core/lib/core/stringpiece.h"
// #include "tensorflow/core/lib/core/threadpool.h"
// #include "tensorflow/core/lib/io/path.h"
// #include "tensorflow/core/lib/strings/stringprintf.h"
// #include "tensorflow/core/platform/env.h"
// #include "tensorflow/core/platform/init_main.h"
// #include "tensorflow/core/platform/logging.h"
// #include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"
// #include "tensorflow/core/util/command_line_flags.h"

#include "prediction/util/status.h"
#include "prediction/server/request.h"
#include "prediction/server/response.h"

namespace prediction {

enum ModuleType {
  FACE_DETECTION_MODULE = 0,
  LOGO_DETECTION_MODULE,
};

class PredictionModule {
 public:
  PredictionModule(std::string module_name, ModuleType module_type,
                   std::string graph_path);

  virtual ~PredictionModule() {}

  tensorflow::Status LoadGraph(const std::string& graph_file_name,
                               std::unique_ptr<tensorflow::Session>* session);

  virtual util::Status Predict(const Request& request, Response* response) {
    return util::Status::OK();
  }

  std::string module_name() { return module_name_; }
  ModuleType module_type() { return module_type_; }

  std::unique_ptr<tensorflow::Session>& session() {
    return session_;
  }

 private:
  std::string module_name_;
  ModuleType module_type_;
  std::string graph_path_;
  std::unique_ptr<tensorflow::Session> session_;
};  // PredictionModule

}  // namespace prediction
#endif  // PREDICTION_MODULE_PREDICTION_MODULE_H_

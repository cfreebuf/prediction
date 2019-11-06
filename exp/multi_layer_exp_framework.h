#ifndef MULTI_LAYER_EXP_FRAMEWORK_H_
#define MULTI_LAYER_EXP_FRAMEWORK_H_

#include <map>
#include <mutex>
#include <memory>
#include "request_context.h"
#include "experiment_config.h"

namespace prediction {

class MultiLayerExpFramework {
 public:
  MultiLayerExpFramework();
  virtual ~MultiLayerExpFramework();

  int LoadConfig(const std::string & exp_path);
  int ReloadConfig(const std::string & exp_path);

  int DoExp(const RequestContext& request,
            std::list<Experiment*>& experiments,
            Parameters& parameters);

  std::shared_ptr<ExperimentConfig> GetConfig();

 private:
  std::vector<std::shared_ptr<ExperimentConfig>> _exp_configs;
  int _current_config_index;

  std::mutex _mutex;
};

}

#endif /* MULTI_LAYER_EXP_FRAMEWORK_H_ */

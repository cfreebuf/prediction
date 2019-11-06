#include "multi_layer_exp_framework.h"
#include "atomic.h"

#ifndef LIB_VERSION
#define LIB_VERSION "1.0.0"
#endif

namespace prediction {

MultiLayerExpFramework::MultiLayerExpFramework() : _current_config_index(0) {
    _exp_configs.push_back(std::make_shared<ExperimentConfig>());
    _exp_configs.push_back(std::make_shared<ExperimentConfig>());
}

MultiLayerExpFramework::~MultiLayerExpFramework() {
}

int MultiLayerExpFramework::LoadConfig(const std::string & exp_path) {
    return ReloadConfig(exp_path);
}

int MultiLayerExpFramework::ReloadConfig(const std::string & exp_path) {
    std::shared_ptr<ExperimentConfig> new_config = std::make_shared<ExperimentConfig>();
    int ret = new_config->LoadConfig(exp_path);
    if (ret != 0) { return -1; }

    std::unique_lock<std::mutex> lock(_mutex);
    _current_config_index = 1 - _current_config_index;
    _exp_configs[_current_config_index] = new_config;
    return 0;
}

std::shared_ptr<ExperimentConfig> MultiLayerExpFramework::GetConfig() {
    std::unique_lock<std::mutex> lock(_mutex);
    return _exp_configs[_current_config_index];
}

int MultiLayerExpFramework::DoExp(const RequestContext& request,
        std::list<Experiment*>& experiments,
        Parameters& parameters) {
    std::shared_ptr<ExperimentConfig> exp_config = GetConfig();
    if (exp_config == NULL) { return -1; }

    return exp_config->DoExp(request, experiments, parameters);
}

}

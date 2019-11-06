#ifndef EXPERIMENT_CONFIG_H_
#define EXPERIMENT_CONFIG_H_

#include <string>
#include <vector>
#include <list>
#include <set>
#include <memory>

#include "experiment_config_meta.h"
#include "element_registry.h"
#include "exp_experiment_base.h"
#include "experiment_elements.h"
#include "atomic.h"

namespace prediction {

class ConfigLoadHelper;

class ExperimentConfig {
 public:
  ExperimentConfig();
  virtual ~ExperimentConfig();

  int DoExp(const RequestContext& request,
            std::list<Experiment*>& experiments,
            Parameters & parameters);

 public:
  int LoadConfig(const std::string& config_path);

  void IncRefCount() { atomic_inc(&_ref_count); }
  void DecRefCount() { atomic_dec(&_ref_count); }
  uint64_t GetRefCount() { return _ref_count; }

 private:
  int BuildConfig(RootEntryConfigMeta& root_entry);
  int BuildDomain(DomainConfigMeta& domain_meta, Domain& domain, int bucket_start);
  int BuildLayer(const LayerConfigMeta& layer_meta, Layer & layer);
  int BuildExperiment(const ExpConfigMeta& exp_meta, Experiment& experiment,
                      const std::string& layer_name, int bucket_start);

  int ConditionChecker(std::vector<Condition>& condition_vec,
                       std::vector<int>& condition_range,
                       const LayerConfigMeta& layer_meta,
                       const ExpConfigMetaBasePtr& exp_meta,
                       int& bucket_start);
  int CheckLayer(const Layer& layer, Domain& domain);

  int ValidateConfig();

 private:
  ElementRegistry<Domain> _domain_registry;
  ElementRegistry<Layer> _layer_registry;
  ElementRegistry<Experiment> _experiment_registry;
  Parameters _default_params;

  // std::vector<std::shared_ptr<Layer>> _launch_layers;
  // std::vector<std::shared_ptr<Domain>> _domains;
  std::vector<Layer*> _launch_layers;
  std::vector<Domain*> _domains;

  int _root_hash_base;
  std::string _root_hash_seed;

  uint64_t _ref_count;

  std::string _config_path;

  std::shared_ptr<ConfigLoadHelper> _config_load_helper;
};

} /* namespace prediction */

#endif /* EXPERIMENT_CONFIG_H_ */

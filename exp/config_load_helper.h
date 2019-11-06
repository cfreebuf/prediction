#ifndef CONFIG_LOAD_HELPER_H_
#define CONFIG_LOAD_HELPER_H_

#include <libconfig.h++>
#include "experiment_config_meta.h"

namespace prediction {

class Parameters;

static const std::string ROOT_ENTRY = "root.entry";
static const std::string LAYER_SUFFIX = ".layer";
static const std::string DOMAIN_SUFFIX = ".domain";
static const std::string DEFAULT_PARAMS = "default.parameters";

class ConfigLoadHelper {
 public:
  ConfigLoadHelper();
  virtual ~ConfigLoadHelper();

  int LoadRootEntry(RootEntryConfigMeta & root_meta);
  int LoadDefaultParams(Parameters&);

  void SetConfigPath(std::string config_path) { _config_path = config_path; }

 protected:
  friend class ExperimentConfig;

 private:
  int LoadLayerFromFile(const std::string & layer_file, LayerConfigMeta & layer_meta);
  int LoadDomainFromElement(libconfig::Setting & domain_element, DomainConfigMeta & domain_meata);
  int AddExpElement(libconfig::Setting & exp_element, LayerConfigMeta & layer_meta);
  void Setting2Str(libconfig::Setting & param,std::string& val);

 private:
  std::string _config_path;
};

} // namespace prediction
#endif // CONFIG_LOAD_HELPER_H_

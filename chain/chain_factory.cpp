// CopyRight 2019 360. All rights reserved.
// File   chain_factory.cpp
// Date   2019-10-29 11:13:32
// Brief

#include "util/logging.h"
#include "util/file_utils.h"
#include "util/string_utils.h"
#include "chain/factory_context.h"
#include "chain/chain_factory.h"

namespace prediction {

using namespace util;

ChainFactory::ChainFactory(FactoryContext* factory_context)
    : factory_context_(factory_context) {
}

ChainFactory::~ChainFactory() {
  for (auto& kv : chains_) {
    if (kv.second) {
      delete kv.second;
    }
  }
}

int ChainFactory::LoadProcessors(const libconfig::Setting& chain_root,
                                 ChainConfigItem* item) {
  try {
    libconfig::Setting& processors = chain_root["processors"];
    if (processors.getType() != libconfig::Setting::TypeArray
        || processors.getLength() < 0) {
      LOG(FATAL) << "processors should be an array chain:" << item->chain_name;
      return -1;
    }
    if (processors[0].getType() != libconfig::Setting::TypeString) {
      LOG(FATAL) << "processors should be string chain:" << item->chain_name;
      return -1;
    }
    int size = processors.getLength();
    for (int i = 0; i < size; i++) {
      item->processors.push_back((const char*) processors[i]);
    }
  } catch (libconfig::SettingNotFoundException& e) {
    LOG(FATAL) << "no processors defined chain:" << item->chain_name;
    return -1;
  } catch (...) {
    LOG(FATAL) << "unknown error when read processors chain:" << item->chain_name;
    return -1;
  }

  try {
    libconfig::Setting& processor_params = chain_root["processor_params"];
    if (processor_params.getType() != libconfig::Setting::TypeGroup
        || processor_params.getLength() < 1) {
      LOG(INFO) << "no processor_params chain:" << item->chain_name;
      return 0;
    }
    int params_size = processor_params.getLength();
    for (int i = 0; i < params_size; ++i) {
      libconfig::Setting & param_setting = processor_params[i];
      std::string param_name = param_setting.getName();
      libconfig::Setting::Type param_type = param_setting.getType();
      Variable val;
      switch (param_type) {
        case libconfig::Setting::TypeBoolean:
          val = (bool) param_setting;
          break;

        case libconfig::Setting::TypeInt64:
        case libconfig::Setting::TypeFloat:
          val = (double) param_setting;
          break;

        case libconfig::Setting::TypeInt:
          val = (int) param_setting;
          break;

        case libconfig::Setting::TypeString:
          val = std::string((const char *) param_setting);
          break;
        case libconfig::Setting::TypeArray:
        case libconfig::Setting::TypeGroup:
        case libconfig::Setting::TypeList:
        case libconfig::Setting::TypeNone:
        default:
          break;
      }
      item->processor_params[param_name] = val;
    }
  } catch (libconfig::SettingNotFoundException& e) {
    LOG(INFO) << "no processor_params chain:" << item->chain_name;
  } catch (...) {
    LOG(FATAL) << "parse processor_params exception chain:" << item->chain_name;
  }
  return 0;
}

// Load renderer and processor in chain_root
int ChainFactory::LoadChain(const libconfig::Setting& chain_root,
                            ChainConfigItem* item) {
  item->chain_name = chain_root.getName();

  // renderer
  std::string renderer_name;
  if (chain_root.lookupValue(std::string("renderer"), renderer_name)) {
    item->renderer = renderer_name;
  }

  int processor_err = LoadProcessors(chain_root, item);
  return processor_err;
}

// Load chains config in chain_conf_file
int ChainFactory::LoadChain(const std::string& chain_conf_file,
                            ChainConfigItem* item) {
  libconfig::Config config;
  try {
    config.readFile(chain_conf_file.c_str());
  } catch (std::exception& e) {
    LOG(FATAL) << "load chain " << chain_conf_file << " error:" << e.what();
    return -1;
  }

  std::string chain_base_name = FileUtils::BaseName(chain_conf_file);

  const libconfig::Setting& root = config.getRoot();
  if (root.getType() != libconfig::Setting::TypeGroup
      || root.getLength() < 1) {
    LOG(FATAL) << "invalid chain config:" << chain_conf_file;
    return -1;
  }

  const libconfig::Setting& chain_root = root[0];
  if (chain_root.getType() != libconfig::Setting::TypeGroup
      || chain_root.getLength() < 1) {
    LOG(FATAL) << "invalid chain config:" << chain_conf_file;
    return -1;
  }

  const std::string& chain_name = chain_root.getName();
  if (chain_name != chain_base_name) {
    LOG(FATAL) << "file name: " << chain_base_name
               << " chain name:" << chain_name << " not match";
    return -1;
  }
  return LoadChain(chain_root, item);
}

Chain* ChainFactory::CreateChainInstance(const ChainConfigItem& item) {
  Chain* chain = new Chain();
  chain->set_chain_name(item.chain_name);

  int err = 0;
  for (const std::string& processor_name : item.processors) {
    Processor* processor =
      factory_context_->processor_factory->GetProcessor(processor_name);
    if (processor == NULL) {
      LOG(FATAL) << "cannot find processor:" << processor_name
                 << "chain:" << item.chain_name;
      err = 1;
      break;
    }
    processor->set_context(factory_context_);
    chain->AddProcessor(processor);
  }

  if (!item.processor_params.empty()) {
    chain->SetProcessorParams(item.processor_params);
  }

  if (!item.renderer.empty()) {
    Renderer* renderer =
      factory_context_->renderer_factory->GetRenderer(item.renderer);
    if (renderer == NULL) {
      LOG(FATAL) << "cannot find renderer:" << item.renderer
                 << "chain:" << item.chain_name;
      err = 1;
    } else {
      chain->set_renderer(renderer);
    }
  }

  if (err != 0) {
    delete chain;
    chain = NULL;
  }
  return chain;
}

int ChainFactory::LoadConfig(const std::string& chain_config_path) {
  std::vector<std::string> file_list = FileUtils::ListDir(chain_config_path);
  std::map<std::string, ChainConfigItem> chain_configs;

  // load all config files first
  for (size_t i = 0; i < file_list.size(); i++) {
    std::string& file_name = file_list[i];
    if (!StringUtils::EndsWith(file_name, ".conf")) {
      continue;
    }
    ChainConfigItem item;
    if (LoadChain(file_name, &item) != 0) {
      LOG(FATAL) << "load chain config erorr path:" << chain_config_path
                 << "file:" << file_name;
      return -1;
    }
    chain_configs[item.chain_name] = item;
  }

  // config file loaded, now we need to convert string to object
  // load all chains ...
  for (const auto& kv : chain_configs) {
    std::vector<std::shared_ptr<Chain>> chain_vec;
    Chain* chain = CreateChainInstance(kv.second);
    if (chain) {
      chains_[kv.first] = chain;
    } else {
      LOG(FATAL) << "load chain " << kv.first << " error";
      return -1;
    }
  }

  return 0;
}

Chain* ChainFactory::GetChain(const std::string& chain_name) {
  Chain* chain = NULL;

  const auto it = chains_.find(chain_name);
  if (it != chains_.end()) {
    chain = it->second;
  }
  return chain;
}

}  // namespace prediction

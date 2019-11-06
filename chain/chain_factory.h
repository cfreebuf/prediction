// File   chain_factory.h
// Author lidongming
// Date   2018-09-25 22:28:33
// Brief

#ifndef PREDICTION_SERVER_CHAIN_CHAIN_FACTORY_H_
#define PREDICTION_SERVER_CHAIN_CHAIN_FACTORY_H_

#include <libconfig.h++>
#include "chain/chain.h"

namespace prediction {

class ChainConfigItem {
 public:
  std::string chain_name;
  std::string renderer;
  std::vector<std::string> processors;
  std::map<std::string, Variable> processor_params;
};

class ProcessorFactory;
class RendererFactory;
class FactoryContext;

class ChainFactory {
 public:
  explicit ChainFactory(FactoryContext*);
  virtual ~ChainFactory();
  int LoadConfig(const std::string& chain_config_path);

  void Clear();
  Chain* GetChain(const std::string& chain_name);

  FactoryContext* factory_context() { return factory_context_; }

 private:
  int LoadChain(const std::string& chain_conf_file, ChainConfigItem* item);
  int LoadChain(const libconfig::Setting& chain_root, ChainConfigItem* item);
  int LoadProcessors(const libconfig::Setting& chain_root, ChainConfigItem* item);

  Chain* CreateChainInstance(const ChainConfigItem& item);

 private:
  FactoryContext* factory_context_;
  std::map<std::string, Chain*> chains_;
};  // ChainFactory

}  // namespace prediction

#endif  // PREDICTION_SERVER_CHAIN_CHAIN_FACTORY_H_

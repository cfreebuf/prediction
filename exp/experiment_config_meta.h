// File   experiment_config_meta.h
// Author lidongming
// Date   2018-09-26 22:41:24
// Brief

#ifndef EXPERIMENT_CONFIG_META_H_
#define EXPERIMENT_CONFIG_META_H_

#include <string>
#include <vector>
#include <list>
#include <map>
#include <memory>

#include "exp/exp_experiment_base.h"
// #include "util/logger.h"
#include <unordered_set>

namespace prediction {

class ExpConfigMetaBase {
 public:
  std::string name;
  int bucket_flow;
  // std::unordered_set<std::string> single_tail_number;
  // std::unordered_set<std::string> mul_tail_number;
  std::map<std::string, std::string> conditions;

 public:
  virtual ExperimentBase::ExperimentType get_type() const = 0;
  virtual void dump() = 0;
  virtual ~ExpConfigMetaBase() { }
  const std::map<std::string, std::string> & get_conditions() const {
      return conditions;
  }
};

class ExpConfigMeta: public ExpConfigMetaBase {
 public:
  std::string owner;
  std::string desc;
  std::string alter_name;
  // int bucket_flow;
  std::map<std::string, std::string> params;

 public:
  void reset();

  void dump();

  virtual ExperimentBase::ExperimentType get_type() const {
      return ExperimentBase::TYPE_EXPERIMENT;
  }
};

class DomainConfigMeta;

typedef std::shared_ptr<ExpConfigMetaBase> ExpConfigMetaBasePtr;
typedef std::shared_ptr<DomainConfigMeta> DomainConfigMetaPtr;
typedef std::shared_ptr<ExpConfigMeta> ExpConfigMetaPtr;

class LayerConfigMeta {
 public:
  std::string name;
  std::string diversion_type;
  int hash_base;
  std::string hash_seed;
  std::vector<ExpConfigMetaBasePtr> experiments;

 public:
  virtual ~LayerConfigMeta();

  void reset() {
      name.clear();
      diversion_type.clear();
      hash_base = 0;
      hash_seed = "";
      experiments.clear();
  }

  void dump();

  ExpConfigMetaPtr add_exp_config();
  DomainConfigMetaPtr add_domain_config();

  int get_hash_base() const{ return hash_base; }
  void set_hash_base(int base) { hash_base = base; }
  void set_hash_seed(const std::string& seed) { hash_seed = seed; }
  const std::string& get_hash_seed() const { return hash_seed; }
};

class DomainConfigMeta: public ExpConfigMetaBase {
 public:
  // int bucket_flow;
  std::string name;
  std::vector<std::string> layer_names;
  std::vector<LayerConfigMeta> layers;

 public:
  void reset() {
      bucket_flow = 0;
      name.clear();
      layer_names.clear();
      layers.clear();
         conditions.clear();
     }

     void dump();

     virtual ExperimentBase::ExperimentType get_type() const {
         return ExperimentBase::TYPE_DOMAIN;
     }
};

class RootEntryConfigMeta {
 public:
  int hash_base;
  std::string hash_seed;
  std::vector<std::string> launcher_layer_names;
  std::vector<LayerConfigMeta> launcher_layers;
  std::vector<DomainConfigMeta> domains;

 public:
  void reset() {
      hash_base = 0;
      hash_seed = "";
      launcher_layer_names.clear();
      domains.clear();
  }

  void dump();
};

}

#endif /* EXPERIMENT_CONFIG_META_H_ */

#ifndef EXP_LAYER_H_
#define EXP_LAYER_H_

#include <list>
#include <string>
#include <vector>
#include <memory>

#include "exp_common.h"
#include "exp_experiment.h"

namespace prediction {

class Layer {
 public:
  Layer() : _div_type(DIV_UID), _hash_base(0) { }
  virtual ~Layer() { }

 public:
  int DoExp(const RequestContext& request,
            std::list<Experiment*>& experiments,
            Parameters& parameters);

  inline std::string GetName() const { return _name; }
  inline void SetName(const std::string & name) { _name = name; }
  inline DiversionType get_div_type() const { return _div_type; }
  void set_div_type(const DiversionType & type) { _div_type = type; }
  void add_experiment(ExperimentBase* p_exp) {
      _exp_list.push_back(p_exp);
  }
  void set_hash_base(const int hash_base) { _hash_base = hash_base; }
  int get_hash_base() const { return _hash_base; }
  void set_hash_seed(const std::string& seed) { _hash_seed = seed; }
  const std::string& get_hash_seed() const { return _hash_seed; }
  const std::vector<ExperimentBase*>& get_experiments() const {
      return _exp_list;
  }


 private:
  ExperimentBase* GetExpByName(const std::string& exp_id);
  ExperimentBase* GetExpByRandom(const RequestContext& request);

 private:
  std::vector<ExperimentBase*> _exp_list;
  std::string _name;
  DiversionType _div_type;
  int _hash_base;
  std::string _hash_seed;
};

} /* namespace prediction */

#endif /* EXP_LAYER_H_ */

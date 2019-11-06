#ifndef EXP_DOMAIN_H_
#define EXP_DOMAIN_H_

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>

#include "exp_condition.h"
#include "exp_experiment.h"
#include "exp_experiment_base.h"

namespace prediction {

class Layer;

class Domain: public ExperimentBase {
 public:
  Domain() { }
  virtual ~Domain() { }

  void AddLayer(Layer* layer) { _layers.push_back(layer); }

  std::vector<Layer*> get_layers() const { return _layers; }

  void SetConditions(const std::map<std::string, std::string>& conditions) {
      _condition.SetConditions(conditions);
  }

  int DoExp(const RequestContext& request, std::list<Experiment*>& experiments,
            Parameters& parameters);

  bool HasExp(const std::string& exp_id) {
      if (_domain_exps.find(exp_id) != _domain_exps.end()) {
          return true;
      }
      return false;
  }
  virtual ExperimentType GetType() const { return TYPE_DOMAIN; }

  const Condition& GetConditions() const { return _condition; }

  const void* GetConditionsPtr() const { return &_condition; }

  virtual bool CheckCondition(const RequestContext & req) {
      return _condition.MatchReq(req);
  }

  std::set<std::string>& GetDomainParams() { return _domain_params; }
  void ResetDomainParams() { _domain_params.clear(); }
  std::set<std::string>& GetDomainExps() { return _domain_exps; }

 private:
  std::vector<Layer*> _layers;
  Condition _condition;
  std::set<std::string> _domain_params; //for check paramenters
  std::set<std::string> _domain_exps; //for check force experiments
};

}

#endif /* EXP_DOMAIN_H_ */

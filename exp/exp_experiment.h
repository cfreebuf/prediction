#ifndef EXP_EXPERIMENT_H_
#define EXP_EXPERIMENT_H_

#include <map>
#include <string>

#include "exp_condition.h"
#include "exp_experiment_base.h"
#include "exp_parameters.h"

namespace prediction {

class Experiment: public ExperimentBase {
 public:
  Experiment();
  virtual ~Experiment();

 public:
  void SetParameter(const std::string& key, const std::string& value) {
      _parameters.set_value(key, value);
  }

  Parameters & GetParameters() { return _parameters; }

  virtual ExperimentType GetType() const { return TYPE_EXPERIMENT; }

  void SetConditions(const std::map<std::string, std::string>& conditions) {
      _condition.SetConditions(conditions);
  }

  const Condition& GetConditions() const { return _condition; }
  const void* GetConditionsPtr() const { return &_condition; }

  virtual bool CheckCondition(const RequestContext & req) {
      return _condition.MatchReq(req);
  }

  void SetAlterName(const std::string& alter_name) {
      _alter_name = alter_name;
  }

  const std::string& GetAlterName () const {
      return _alter_name;
  } 

 private:
  std::string _alter_name;
  Condition _condition;
  Parameters _parameters;
};

}

#endif /* EXP_EXPERIMENT_H_ */

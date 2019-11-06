// CopyRight 2019 360. All rights reserved.
// File   exp_condition.h
// Date   2019-10-28 15:00:24
// Brief

#ifndef EXP_CONDITION_H_
#define EXP_CONDITION_H_

#include <map>
#include <string>
#include <memory>
#include "regex_util.h"

namespace prediction {

class RequestContext;

class Condition {
 public:
  /*
   *  @return:
   *     -1: condition confliction
   *      0: same dimension, but not equal
   *      1: equal
   *      2: lhs has a higher priority to rhs
   *      3: rhs has a higher priority to lhs
   */
  enum ConditionStatus {
      CONFLICTION = -1,
      SAME_DIMENSION = 0,
      EQUAL = 1,
      LARGER = 2,
      SMALLER = 3
  };

  Condition();
  Condition(const std::map<std::string, std::string>& conditions);
  Condition(const Condition& rhs);
  virtual ~Condition();

  ConditionStatus IsSameDimension(const Condition& rhs_condition);

  inline std::string GetName() const { return _name; }
  inline void SetName(const std::string& name) { _name = name; }

  void SetCondition(const std::string& key, const std::string& value);
  void SetConditions(const std::map<std::string, std::string>& conditions);
  std::string GetCondition(const std::string& key);
  std::shared_ptr<Regex> GetConditionRegex(const std::string& key);

  const std::string ToString() const;

  bool MatchReq(const RequestContext& req);

 private:
  std::map<std::string, std::string> _conditions;
  std::map<std::string, std::shared_ptr<Regex>> _condition_ptrs;
  std::string _name;

};

}
#endif // EXP_CONDITION_H_

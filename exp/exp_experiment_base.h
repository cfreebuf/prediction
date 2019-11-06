#ifndef EXP_EXPERIMENT_BASE_H_
#define EXP_EXPERIMENT_BASE_H_

#include <string>
#include "request_context.h"
#include <unordered_set>
namespace prediction {

class ExperimentBase {
 public:
  enum ExperimentType {
      TYPE_UNKNOWN = 0,
      TYPE_EXPERIMENT = 1,
      TYPE_DOMAIN = 2,
  };

  ExperimentBase() : _name("default_exp"), _bucket_flow(-1), _bucket_start(0), _bucket_end(0) { }
  virtual ~ExperimentBase() { }

  inline std::string GetName() const { return _name; }
  inline void SetName(const std::string & name) { _name = name; }
  bool InRange(int bucket) const {
      return (_bucket_flow > 0)
          && (bucket >= _bucket_start && bucket <= _bucket_end);
  }

  void SetBucketStart(const int bucket_start) { _bucket_start = bucket_start; }
  int GetBucketStart() const { return _bucket_start; }
  void SetBucketEnd(const int bucket_end) { _bucket_end = bucket_end; }
  int GetBucketEnd() const { return _bucket_end; }
  void SetBucketFlow(const int bucket_flow) { _bucket_flow = bucket_flow; }
  int GetBucketFlow() const { return _bucket_flow; }
  virtual ExperimentType GetType() const { return TYPE_UNKNOWN; }

  virtual bool CheckCondition(const RequestContext & req) = 0;
  virtual const void* GetConditionsPtr() const= 0; 


 protected:
  std::string _name;
  int _bucket_flow;
  int _bucket_start;
  int _bucket_end;

#if 0
  // remove
  std::unordered_set<std::string> _single_tail_number;
  std::unordered_set<std::string> _mul_tail_number;

  // FIXME(lidongming):remove these codes
  bool in_range(std::string single_tail_number,
          std::string mul_tail_number) const {
      if (_single_tail_number.empty() && _mul_tail_number.empty()) {
          return true;
      }
      if (single_tail_number.empty() && mul_tail_number.empty()) {
          return false;
      }
      if (_single_tail_number.find(single_tail_number) != _single_tail_number.end()
              || _mul_tail_number.find(mul_tail_number) != _mul_tail_number.end()) {
          return true;
      }
      return false;
  }
  void set_single_tail_number(const std::unordered_set<std::string>& single_tail_number) {
      _single_tail_number = single_tail_number;
  }
  std::unordered_set<std::string>& get_single_tail_number() {
      return _single_tail_number;
  }
  void set_mul_tail_number(const std::unordered_set<std::string>& mul_tail_number) {
      _mul_tail_number = mul_tail_number;
  }
  std::unordered_set<std::string>& get_mul_tail_number() {
      return _mul_tail_number;
  }
#endif
};

}

#endif /* EXP_EXPERIMENT_BASE_H_ */

#ifndef EXP_PARAMETERS_H_
#define EXP_PARAMETERS_H_

#include <map>
#include <string>
#include <utility>

namespace prediction {

class Parameters {
 public:
  Parameters() { }

  virtual ~Parameters() { }

  void reset() {
      _params.clear();
      _params_source.clear();
  }

  bool has_key(const std::string& key) const {
      auto it = _params.find(key);
      if (it == _params.end()) { return false; }
      else { return true; }
  }

  std::string get_value(const std::string& key) const {
      auto it = _params.find(key);
      if (it == _params.end()) { return ""; }
      else { return it->second; }
  }

  const std::string get_source(const std::string& key) const {
      auto it = _params_source.find(key);
      if (it == _params_source.end()) { return ""; }
      else { return it->second; }
  }

  void set_value(const std::string& key, const std::string& value) {
      _params[key] = value;
  }

  void set_source(const std::string& key, const std::string& value) {
      _params_source[key] = value;
  }

  const std::map<std::string, std::string>& get_params() const {
      return _params;
  }

  const std::map<std::string, std::string>& get_params_source() const {
      return _params_source;
  }

  void dump();

 private:
  std::map<std::string, std::string> _params;
  std::map<std::string, std::string> _params_source;


};

}

#endif /* EXP_PARAMETERS_H_ */

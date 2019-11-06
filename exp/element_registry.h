#ifndef ELEMENT_REGISTRY_H_
#define ELEMENT_REGISTRY_H_

#include <map>
#include <string>
#include <memory>

template<typename T>
class ElementRegistry {
 private:
  std::map<std::string, T> _registry;

 public:
  int add(const T & t) {
      std::string name = t.GetName();
      if (_registry.find(name) != _registry.end()) {
          return -1; // TODO: return a concrete error code, ALREADY_EXIST
      }
      _registry.insert(std::make_pair(name, t));

      return 0;
  }

  int remove(const std::string & name) {
      auto it = _registry.find(name);
      if (it == _registry.end()) {
          return -1;
      } else {
          _registry.remove(it);
          return 0;
      }
  }

  T * get_element(const std::string & name) {
      auto it = _registry.find(name);
      if (it == _registry.end()) {
          return nullptr;
      } else {
          return &(it->second);
      }
  }

#if 0
  std::shared_ptr<T> get_element(const std::string & name) {
      auto it = _registry.find(name);
      if (it == _registry.end()) {
          return nullptr;
      } else {
          // std::shared_ptr<T> p = std::make_shared<T>();
          std::shared_ptr<T> p;
          p.reset(&(it->second));
          return p;
      }
  }
#endif
  std::map<std::string, T> & get_all_elements() {
      return _registry;
  }
};

#endif /* ELEMENT_REGISTRY_H_ */

// File   component_factory.h
// Author lidongming
// Date   2018-09-26 20:09:15
// Brief

#ifndef PREDICTION_SERVER_PROCESSOR_COMPONENT_FACTORY_H_
#define PREDICTION_SERVER_PROCESSOR_COMPONENT_FACTORY_H_

namespace prediction {

class ComponentFactory {
 public:
  ComponentFactory(void* context) : _context(context) {  }
  ~ComponentFactory() {  }

  void SetContext(void* context) { _context = context; }
  void* GetContext() { return _context; }

 protected:
  void* _context;
};  // ComponentFactory

}  // namespace prediction

#endif  // PREDICTION_SERVER_PROCESSOR_COMPONENT_FACTORY_H_

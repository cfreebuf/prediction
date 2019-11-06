// File   processor_factory.h
// Author lidongming
// Date   2018-09-26 20:06:28
// Brief

#ifndef PREDICTION_SERVER_PROCESSOR_PROCESSOR_FACTORY_H_
#define PREDICTION_SERVER_PROCESSOR_PROCESSOR_FACTORY_H_

#include <string>
#include <map>
#include "processor/component_factory.h"

namespace prediction {

class Processor;

class ProcessorFactory : public ComponentFactory {
 public:
  ProcessorFactory(void* context);
  ~ProcessorFactory();

  void Init();
  int RegisterProcessor(const std::string name, Processor* processor);
  Processor* GetProcessor(const std::string& name);

 private:
  // DISALLOW_COPY_AND_ASSIGN(ProcessorFactory);
  std::map<std::string, Processor*> processor_mapping_;
};  // ProcessorFactory

}  // namespace prediction

#endif  // PREDICTION_SERVER_PROCESSOR_PROCESSOR_FACTORY_H_

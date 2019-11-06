// File   processor_factory.cpp
// Author lidongming
// Date   2018-09-26 20:04:30
// Brief

#include "processor/processor_factory.h"
#include "processor/processors.h"
#include "chain/execution.h"
// #include "util/logging.h"

namespace prediction {

ProcessorFactory::ProcessorFactory(void* context) : ComponentFactory(context) {
    Init();
}
ProcessorFactory::~ProcessorFactory() { }

#define REGISTER_PROCESSOR(_processor_name_, _processor_class_) \
  RegisterProcessor(#_processor_name_, new _processor_class_())


void ProcessorFactory::Init() {
  REGISTER_PROCESSOR(predict_processor, PredictProcessor);
}

int ProcessorFactory::RegisterProcessor(const std::string name,
    Processor* processor) {
  if (processor_mapping_.count(name) > 0) {
    // LOG(WARNING) << "processor already registered:" << name;
    return -1;
  } else {
    processor_mapping_[name] = processor;
    return 0;
  }
}

Processor* ProcessorFactory::GetProcessor(const std::string& name) {
  auto it = processor_mapping_.find(name);
  if (it != processor_mapping_.end()) {
    return it->second;
  }
  return NULL;
}

}

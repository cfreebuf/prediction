// File   execution.cpp
// Author lidongming
// Date   2018-09-25 20:40:23
// Brief

#include "prediction/chain/execution.h"

namespace prediction {

Execution::Execution() { }
Execution::~Execution() { }

Execution::Execution(const Chain& processor_chain) {
  chain_ = processor_chain;
}

Execution::Execution(const Chain* chain) {
  if (chain!= NULL) {
    chain_ = *chain;
  }
}

int Execution::Execute(Context& context, Processor* current) {
  Processor* next = chain_.GetNext(current);
  if (next != NULL) {
    next->Process(context);
  }

  return 0;
}

}  // namespace prediction

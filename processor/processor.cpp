// File   processor.cpp
// Author lidongming
// Date   2018-09-26 20:10:13
// Brief

#include "processor/processor.h"
#include "chain/execution.h"

namespace prediction {

// Processor
int Processor::CallNextProcessor(Context& context) {
    Execution* execution = context.execution();
    if (execution) {
      return execution->Execute(context, this);
    } else {
      return -1;
    }
}

}

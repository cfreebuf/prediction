// File   execution.h
// Author lidongming
// Date   2018-09-25 20:38:58
// Brief

#ifndef PREDICTION_SERVER_CHAIN_EXECUTION_H_
#define PREDICTION_SERVER_CHAIN_EXECUTION_H_

#include "prediction/chain/chain.h"
#include "prediction/server/result.h"

namespace prediction {

class Execution {
 public:
  Execution();
  // Copy
  Execution(const Chain& chain);
  Execution(const Chain* chain);
  virtual ~Execution();

  virtual int Execute(Context& context, Processor*);

  Chain& chain() { return chain_; }

 private:
  Chain chain_;

};  // Execution

}  // namespace prediction

#endif  // PREDICTION_SERVER_FRAMEWORK_EXECUTION_H_

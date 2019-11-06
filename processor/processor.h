// CopyRight 2019 360. All rights reserved.
// File   processor.h
// Date   2019-10-29 11:19:16
// Brief

#ifndef PREDICTION_SERVER_PROCESSOR_PROCESSOR_H_
#define PREDICTION_SERVER_PROCESSOR_PROCESSOR_H_

#include <string>
#include "chain/context.h"
#include "util/logging.h"

namespace prediction {

class Processor {
 public:
  virtual int Process(Context& context) {
    LOG(INFO) << "In base process rid:" << context.request()->rid();
    return 0;
  };
  virtual ~Processor() {};
  virtual const std::string name() const = 0;

  virtual int CallNextProcessor(Context& context);

  void* context() { return _context; }
  void set_context(void* context) { _context = context; }

 protected:
  void* _context;
};  // Processor

}  // namespace prediction

#endif  // PREDICTION_SERVER_PROCESSOR_PROCESSOR_H_

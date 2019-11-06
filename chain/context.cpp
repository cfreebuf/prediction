// File   context.cpp
// Author lidongming
// Date   2018-09-25 20:44:10
// Brief

#include "prediction/chain/context.h"
#include "prediction/common/errcode.h"

namespace prediction {

Context::Context(Request* request, Result* result, Execution* _exec)
    : request_(request), result_(result), execution_(_exec),
     err_code_(PREDICT_OK) {
}

Context::~Context() { }

}  // namespace prediction

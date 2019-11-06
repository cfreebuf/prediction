// CopyRight 2019 360. All rights reserved.
// File   default_renderer.cpp
// Date   2019-10-28 14:36:45
// Brief

#include "prediction/renderer/default_renderer.h"
#include "prediction/chain/context.h"
#include "prediction/common/common_gflags.h"
#include "prediction/util/monitor.h"
#include "prediction/util/time_utils.h"

namespace prediction {

using namespace util;

int DefaultRenderer::Render(Context& context) {
  int err_code = context.err_code();
  int result_count = 0;
  if (err_code == 0) {
  }

  // LOG(INFO) << "[RESPONSE] rid:" << request->rid
  //           << " result_count:" << result_count
  //           << " render_latency:" << render_latency;
  return 0;
}

}  // namespace pediction

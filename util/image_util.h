// CopyRight 2019 360. All rights reserved.
// File   image_util.h
// Date   2019-11-05 22:02:36
// Brief

#ifndef PREDICTION_UTIL_IMAGE_UTIL_H_
#define PREDICTION_UTIL_IMAGE_UTIL_H_

#include <string>
#include "prediction/util/status.h"

namespace prediction {

extern util::Status DownloadImageFromURL(std::string url, std::string output);

}  // namespace prediction
#endif  // PREDICTION_UTIL_IMAGE_UTIL_H_

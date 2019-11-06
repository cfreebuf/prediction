// CopyRight 2019 360. All rights reserved.
// File   prediction.h
// Date   2019-10-29 11:04:20
// Brief

#ifndef PREDICTION_SERVER_PREDICTION_H_
#define PREDICTION_SERVER_PREDICTION_H_

#include <string>

namespace prediction {

struct Prediction {
    std::string prediction_id;
    std::string model_id;
};  // Prediction

}  // namespace prediction

#endif  // PREDICTION_SERVER_PREDICTION_H_

// CopyRight 2019 360. All rights reserved.
// File   face_detection_test.cpp
// Date   2019-11-04 20:01:41
// Brief

#include <iostream>
#include <opencv2/opencv.hpp>
#include "prediction/third_party/gtest/include/gtest/gtest.h"
#include "prediction/module/logo_detection_module.h"
#include "prediction/server/tensorflow_serving_client.h"
#include "prediction/util/status.h"
#include "prediction/util/logging.h"
#include "prediction/util/image_util.h"
#include "prediction/common/common_gflags.h"

using namespace prediction;

class LogoDetectionTest: public ::testing::Test {
 public:
  void SetUp() { }
  void TearDown() { }
};

TEST_F(LogoDetectionTest, TestLogoDetect) {
  Request request;
  Response response;
  std::string rid = request.rid();
  response.set_rid(rid);

  std::string url = FLAGS_logo_detection_req_image_url;
  std::string image_file = FLAGS_logo_detection_req_path + rid + ".jpg";
  util::Status status = DownloadImageFromURL(url, image_file);
  if (!status.ok()) {
    LOG(WARNING) << status.ToString();
    return;
  } else {
    LOG(INFO) << "Download image successfully url:"
              << FLAGS_logo_detection_req_image_url;
  }

  request.AddMember("image_path", image_file);
  request.Parse();
  LOG(INFO) << request.ToString();

  LogoDetectionModule module(FLAGS_logo_model);
  if (module.Predict(request, &response) != 0) {
    LOG(WARNING) << "Predict error";
    return;
  }
  LOG(INFO) << response.ToString();

#if 0
  if (TensorflowServingClient::Init() != 0) {
    LOG(FATAL) << "Failed to init tensorflow serving client";
  }
#endif
  // std::string image_file = "./test/data/logo.jpg";
  // cv::Mat frame = cv::imread(image_file);
  // if (!frame.data) {
  //   LOG(WARNING) << "Failed to read image file: " << image_file;
  // }
  // LogoDetectionModule module;
  // module.Init(FLAGS_logo_model);
  // module.Detect(frame);

#if 0
  LOG(INFO) << "Face size:" << face_boxes.size();
  for (int i = 0; i < face_boxes.size(); i++) {
    FaceBox& face_box = face_boxes[i];
    LOG(INFO) << "FACE " << i << " x0:" << face_box.x0 << " y0:" << face_box.y0
              << " x1:" << face_box.x1 << " y1:" << face_box.y1
              << " score:" << face_box.score;
    cv::Mat face_frame = frame.clone();
    module.DrawRectAndLandmark(face_frame, face_box);
    module.SaveImage("face_" + std::to_string(i) + ".jpg", face_frame);
  }
#endif
  // EXPECT_EQ(parser.parse_status(), Status::OK());
}

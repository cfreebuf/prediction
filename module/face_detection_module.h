// CopyRight 2019 360. All rights reserved.
// File   face_detection_module.h
// Date   2019-11-05 01:07:52
// Brief

#ifndef PREDICTION_MODULE_FACE_DETECTION_MODULE_H_
#define PREDICTION_MODULE_FACE_DETECTION_MODULE_H_

#include <string>
#include <vector>
#include <opencv2/opencv.hpp>
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/public/session.h"
#include "prediction/module/prediction_module.h"

namespace prediction {

struct FaceLandmark {
	float x[5];
	float y[5];
};

struct FaceBox {
	float x0;
	float y0;
	float x1;
	float y1;
	float score;
	float regress[4];
	float px0;
	float py0;
	float px1;
	float py1;
	FaceLandmark landmark;  
};

struct ScaleWindow {
	int h;
	int w;
	float scale;
};

class FaceDetectionModule : public PredictionModule {
 public:
  FaceDetectionModule(std::string graph_path);
  ~FaceDetectionModule() {}

  int PNet(cv::Mat& img, const ScaleWindow& win,
           std::vector<FaceBox>* output_boxes);
  int RNet(cv::Mat& img, std::vector<FaceBox>& pnet_boxes,
           std::vector<FaceBox>* output_boxes);
  int ONet(cv::Mat& img, std::vector<FaceBox>& rnet_boxes,
           std::vector<FaceBox>* output_boxes);

  util::Status Predict(const Request& request, Response* response) override;

  void GenerateBoundingBox(const float* confidence_data, const float* reg_data,
                           float scale, float threshold, int h, int w,
                           bool transposed, std::vector<FaceBox>* output);

  void PyramidScales(int height, int width, int min_size, float factor,
                     std::vector<ScaleWindow>* ScaleWindows);

  void ProcessBoxes(std::vector<FaceBox>& input, int img_h, int img_w,
                    std::vector<FaceBox>* res);
  void Normalize(std::vector<FaceBox>& input, float threshold, int type,
                 std::vector<FaceBox>* output);
  void Regress(std::vector<FaceBox>* boxes);
  void Square(std::vector<FaceBox>* boxes);
  void Padding(int img_h, int img_w, std::vector<FaceBox>* res);

  void Patch(const cv::Mat& img, FaceBox& box, float* data_to, int height, int width);

  void DrawRectAndLandmark(cv::Mat& frame, FaceBox& box);
  // void SaveImage(std::string file_name, cv::Mat& frame);

 private:
};

}  // namespace prediction
#endif  // PREDICTION_MODULE_FACE_DETECTION_MODULE_H_

// CopyRight 2019 360. All rights reserved.
// File   logo_detection_module.cpp
// Date   2019-11-05 09:44:46
// Brief

#include <fstream>
#include <utility>
#include <vector>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>

#include "tensorflow/cc/ops/const_op.h"
#include "tensorflow/cc/ops/image_ops.h"
#include "tensorflow/cc/ops/standard_ops.h"
#include "tensorflow/core/framework/graph.pb.h"
#include "tensorflow/core/framework/tensor.h"
#include "tensorflow/core/graph/default_device.h"
#include "tensorflow/core/graph/graph_def_builder.h"
#include "tensorflow/core/lib/core/errors.h"
#include "tensorflow/core/lib/core/stringpiece.h"
#include "tensorflow/core/lib/core/threadpool.h"
#include "tensorflow/core/lib/io/path.h"
#include "tensorflow/core/lib/strings/stringprintf.h"
#include "tensorflow/core/platform/env.h"
#include "tensorflow/core/platform/init_main.h"
#include "tensorflow/core/platform/logging.h"
#include "tensorflow/core/platform/types.h"
#include "tensorflow/core/public/session.h"
#include "tensorflow/core/util/command_line_flags.h"

#include "prediction/util/status.h"
#include "prediction/util/logging.h"
#include "prediction/util/time_utils.h"
#include "prediction/common/common_gflags.h"
#include "prediction/module/logo_detection_module.h"

namespace prediction {

using namespace tensorflow;
using tensorflow::Flag;
using tensorflow::Tensor;
// using tensorflow::Status;
using tensorflow::string;
using tensorflow::int32;

LogoDetectionModule::LogoDetectionModule(std::string graph_path)
  : PredictionModule("LogoDetectionModule", LOGO_DETECTION_MODULE, graph_path) {
}

util::Status LogoDetectionModule::Predict(const Request& request, Response* response) {
  const std::unique_ptr<rapidjson::Document>& req_json_doc = request.req_json_doc();
  assert(req_json_doc != nullptr);

  std::string image_path;
  if (req_json_doc->HasMember("input_image_path") && (*req_json_doc)["input_image_path"].IsString()) {
    image_path = (*req_json_doc)["input_image_path"].GetString();
  } else {
    LOG(ERROR) << "No image_path in request or its format is invalid, rid:"
               << request.rid();
    return util::Status(error::NO_INPUT_IMAGE,
                 "No image_path in request or its format is invalid");
  }


  cv::Mat frame = cv::imread(image_path);
  if (!frame.data) {
    LOG(ERROR) << "Read local image error, file: " << image_path;
    return util::Status(error::READ_IMAGE_ERROR, "Read local image error");
  }
  cv::Mat frame_clone = frame.clone();
	cv::Mat img;
  cv::cvtColor(frame, img, cv::COLOR_BGR2RGB);

  int32 image_width = frame.cols;
  int32 image_height = frame.rows;
  float input_mean = 0;
  float input_std = 255;

  // Get the image from disk as a float array of numbers, resized and normalized
  // to the specifications the main graph expects.
  std::vector<Tensor> input_tensors;
  tensorflow::Status read_tensor_status =
      ReadTensorFromImageFile(image_path, image_height, image_width, input_mean,
                              input_std, &input_tensors);
  if (!read_tensor_status.ok()) {
    LOG(ERROR) << read_tensor_status;
    return util::Status(error::READ_TENSOR_FROM_IMAGE_ERROR,
                        read_tensor_status.ToString());
  }
  const Tensor& input_tensor = input_tensors[0];

  std::string input_layer = "image_tensor:0";
  std::vector<std::string> output_layer = {
    "detection_boxes:0",
    "detection_scores:0",
    "detection_classes:0",
    "num_detections:0"
  };
  std::unique_ptr<tensorflow::Session>& session = this->session();
  std::vector<Tensor> outputs;
  tensorflow::Status run_status = session->Run({{input_layer, input_tensor}},
                                   output_layer, {}, &outputs);
  if (!run_status.ok()) {
    LOG(ERROR) << "Running logo detection model failed: "
               << run_status.ToString();
    return util::Status(error::RUNNING_MODEL_ERROR, run_status.ToString());
  }

  auto boxes = outputs[0].flat_outer_dims<float,3>();
  tensorflow::TTypes<float>::Flat scores = outputs[1].flat<float>();
  tensorflow::TTypes<float>::Flat classes = outputs[2].flat<float>();
  tensorflow::TTypes<float>::Flat num_detections = outputs[3].flat<float>();
  // LOG(INFO) << "num_detections:" << num_detections(0) << "," << outputs[0].shape().DebugString();

  bool detected = false;
  float x0, y0, x1, y1;
  for (size_t i = 0; i < num_detections(0) && i < 100; ++i) {
    if (scores(i) > 0.5) {
      y0 = boxes(0,i,0) * image_height;
      x0 = boxes(0,i,1) * image_width;     
      y1 = boxes(0,i,2) * image_height;
      x1 = boxes(0,i,3) * image_width;

      // LOG(INFO) << i << ",score:" << scores(i) << ",class:" << classes(i)
                // << ",x0:" << x0 << ",x1:" << x1 << ",y0:" << y0 << ",y1:" << y1;

      cv::rectangle(frame_clone, cv::Point(x0, y0), cv::Point(x1, y1),
                    cv::Scalar(0, 255, 0), 1);
      detected = true;
    }
  }

  if (detected) {
    uint64_t now = util::TimeUtils::GetCurrentTime();
    std::string save_file_name = FLAGS_logo_detection_res_path
      + request.rid() + "_" + std::to_string(now) + ".jpg";
    cv::imwrite(save_file_name, frame_clone);

    response->AddMember("image", FLAGS_result_http_server + save_file_name);
    response->AddMember("status", "true");
    response->set_success(true);
  } else {
    response->AddMember("status", "false");
    response->set_success(false);
  }

  return util::Status::OK();
}

tensorflow::Status LogoDetectionModule::ReadEntireFile(tensorflow::Env* env,
    const std::string& filename, Tensor* output) {
  tensorflow::uint64 file_size = 0;
  TF_RETURN_IF_ERROR(env->GetFileSize(filename, &file_size));

  std::string contents;
  contents.resize(file_size);

  std::unique_ptr<tensorflow::RandomAccessFile> file;
  TF_RETURN_IF_ERROR(env->NewRandomAccessFile(filename, &file));

  tensorflow::StringPiece data;
  TF_RETURN_IF_ERROR(file->Read(0, file_size, &data, &(contents)[0]));
  if (data.size() != file_size) {
    return tensorflow::errors::DataLoss("Truncated read of '", filename,
                                        "' expected ", file_size, " got ",
                                        data.size());
  }
  // output->scalar<string>()() = data.ToString();
  output->scalar<string>()() = std::move(std::string(data));
  return tensorflow::Status::OK();
}

// Given an image file name, read in the data, try to decode it as an image,
// resize it to the requested size, and then scale the values as desired.
tensorflow::Status LogoDetectionModule::ReadTensorFromImageFile(const std::string& file_name,
    const int image_height, const int image_width, const float input_mean,
    const float input_std, std::vector<Tensor>* out_tensors) {
  auto root = tensorflow::Scope::NewRootScope();
  using namespace ::tensorflow::ops;

  std::string input_name = "file_reader";
  std::string output_name = "normalized";

  // read file_name into a tensor named input
  Tensor input(tensorflow::DT_STRING, tensorflow::TensorShape());
  TF_RETURN_IF_ERROR(
      ReadEntireFile(tensorflow::Env::Default(), file_name, &input));

  // use a placeholder to read input data
  auto file_reader =
      Placeholder(root.WithOpName("input"), tensorflow::DataType::DT_STRING);

  std::vector<std::pair<string, tensorflow::Tensor>> inputs = {
      {"input", input},
  };

  // figure out file kind and decode it.
  const int wanted_channels = 3;
  tensorflow::Output image_reader;
  if (absl::EndsWith(tensorflow::StringPiece(file_name), ".png")){
    image_reader = DecodePng(root.WithOpName("png_reader"), file_reader,
                             DecodePng::Channels(wanted_channels));
  } else if (absl::EndsWith(tensorflow::StringPiece(file_name), ".gif")) {
    // gif decoder returns 4-D tensor, remove the first dim
    image_reader =
        Squeeze(root.WithOpName("squeeze_first_dim"),
                DecodeGif(root.WithOpName("gif_reader"), file_reader));
  } else {
    // Assume if it's neither a PNG nor a GIF then it must be a JPEG.
    image_reader = DecodeJpeg(root.WithOpName("jpeg_reader"), file_reader,
                              DecodeJpeg::Channels(wanted_channels));
  }
  // Cast the image data to float so we can do normal math on it.
  // auto float_caster =
  //     Cast(root.WithOpName("float_caster"), image_reader, tensorflow::DT_FLOAT);
  auto uint8_caster =
      Cast(root.WithOpName("uint8_caster"), image_reader, tensorflow::DT_UINT8);

  // The convention for image ops in TensorFlow is that all images are expected
  // to be in batches, so that they're four-dimensional arrays with indices of
  // [batch, height, width, channel]. Because we only have a single image, we
  // have to add a batch dimension of 1 to the start with ExpandDims().
  auto dims_expander = ExpandDims(root.WithOpName("dim"), uint8_caster, 0);

  // This runs the GraphDef network definition that we've just constructed, and
  // returns the results in the output tensor.
  tensorflow::GraphDef graph;
  TF_RETURN_IF_ERROR(root.ToGraphDef(&graph));

  std::unique_ptr<tensorflow::Session> session(
      tensorflow::NewSession(tensorflow::SessionOptions()));
  TF_RETURN_IF_ERROR(session->Create(graph));
  TF_RETURN_IF_ERROR(session->Run({inputs}, {"dim"}, {}, out_tensors));
  return tensorflow::Status::OK();
}

// void LogoDetectionModule::Detect(cv::Mat& src) {
//   LOG(INFO) << "Start detect logo";
// 	cv::Mat img;
// 	// src.convertTo(img, CV_32FC3);
// 	// img = (img - kMean) * kAlpha;
// 	// img = img.t();
// 	// cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
//   cv::cvtColor(src, img, cv::COLOR_BGR2RGB);
// 
// 	int height = img.rows;
// 	int width = img.cols;
// 
//   LOG(INFO) << "Start input tensor";
//   Tensor input_tensor(DT_UINT8, TensorShape({1, height, width, 3}));
//   uint8_t* input_data_ptr = input_tensor.flat<uint8_t>().data();
//   // cv::Mat fake_mat(height, width, CV_32FC3, input_data_ptr);
//   cv::Mat fake_mat(height, width, CV_8UC3, input_data_ptr);
//   img.convertTo(fake_mat, CV_32FC3);
//   LOG(INFO) << "Finish input tensor";
// 
//   std::vector<std::pair<std::string, Tensor>> inputs;
//   inputs.emplace_back(std::make_pair<std::string, Tensor>("image_tensor",
//         std::move(input_tensor)));
// 
//   std::vector<std::string> output_names = {
//     "detection_boxes:0",
//     "detection_scores:0",
//     "detection_classes:0",
//     "num_detections:0"
//   };
//   LOG(INFO) << "Start run session";
//   std::vector<Tensor> outputs;
//   Status status = session_->Run(inputs, output_names, {}, &outputs);
// 
//   if (!status.ok()) {
//     LOG(WARNING) <<"Logo detect failed:" << status.ToString();
// 		// return -1;
// 	}
// 
//   const Tensor& boxes_tensor  = outputs[0];
//   // boxes_tensor.sequeeze();
//   const TensorShape& boxes_shape  = boxes_tensor.shape();
//   int dims = boxes_shape.dims();
//   LOG(INFO) << "Dims:" << dims;
//   for (int i = 0; i < dims; i++) {
//     LOG(INFO) << "Dim " << i << ":" << boxes_shape.dim_size(i);
//   }
// 
//   tensorflow::TTypes<float>::Flat scores = outputs[1].flat<float>();
//   tensorflow::TTypes<float>::Flat classes = outputs[2].flat<float>();
//   tensorflow::TTypes<float>::Flat num_detections = outputs[3].flat<float>();
//   auto boxes = outputs[0].flat_outer_dims<float,3>();
// 
//   LOG(INFO) << "num_detections:" << num_detections(0) << "," << outputs[0].shape().DebugString();
//   for(size_t i = 0; i < num_detections(0) && i < 20;++i) {
//     if(scores(i) > 0.5) {
//       LOG(INFO) << i << ",score:" << scores(i) << ",class:" << classes(i) <<
//       ",box:" << "," << boxes(0,i,0) << "," << boxes(0,i,1) << "," << boxes(0,i,2)<< "," << boxes(0,i,3);
//     }
//   }
// 
// #if 0
//   const tensorflow::StringPiece& boxes_piece = boxes_tensor.tensor_data();
//   const float* boxes_data = (const float*)boxes_piece.data();
// 
//   int dim0 = boxes_shape.dim_size(0);
//   int dim1 = boxes_shape.dim_size(1);
//   int dim2 = boxes_shape.dim_size(2);
// 
//   float value = 0.0f;
//   int offset = 0;
//   for (int i = 0; i < dim0; i++) {
//     for (int j = 0; j < dim1; j++) {
//       for (int k = 0; k < dim2; k++) {
//         offset = i * dim0 * dim1 * dim2 + k;
//         value = boxes_data[offset];
//         LOG(INFO) << "i:" << i << " j:" << j << " k:" << k << " value:" << value;
//       }
//     }
//   }
// #endif
// #if 0
// 
//   for (int i = 0; i < 300; i++) {
//     LOG(INFO) << "dim value " << i << ":" << boxes_data[i];
//   }
// #endif
// 
// #if 0
//   const Tensor& confidence_tensor  = outputs[1];
// 
//   const TensorShape& regress_shape  = regress_tensor.shape();
// 	const TensorShape& confidence_shape = confidence_tensor.shape();
// 
// 	int h = regress_shape.dim_size(1);
// 	int w = confidence_shape.dim_size(2);
// 
//   const tensorflow::StringPiece& regress_piece = regress_tensor.tensor_data();
//   const tensorflow::StringPiece& confidence_piece = confidence_tensor.tensor_data();
// 
//   const float* regress_data = (const float*)regress_piece.data();
//   const float* confidence_data = (const float*)confidence_piece.data();
// 
//   // int confidence_size = h * w * 2;
// 
// 	std::vector<FaceBox> candidate_boxes;
// 	GenerateBoundingBox(confidence_data, regress_data, scale, kPNetThreshold,
//                       h, w, true, &candidate_boxes);
// 
// 	Normalize(candidate_boxes, 0.5, NMS_UNION, boxes);
// #endif
// }
}  // namespace prediction

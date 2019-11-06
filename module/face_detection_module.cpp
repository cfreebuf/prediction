// CopyRight 2019 360. All rights reserved.
// File   face_detection_module.cpp
// Date   2019-11-05 01:08:42
// Brief

#include <fstream>
#include <utility>
#include <glog/logging.h>
#include <opencv2/imgproc.hpp>
#include "prediction/module/face_detection_module.h"
#include "prediction/common/common_gflags.h"
#include "prediction/util/time_utils.h"

namespace prediction {

const int NMS_UNION = 1;
const int NMS_MIN = 2;
const float kPNetThreshold = 0.6;
const float kRNetThreshold = 0.7;
const float kOnetThreshold = 0.9;

using namespace tensorflow;

FaceDetectionModule::FaceDetectionModule(std::string graph_path)
  : PredictionModule("FaceDetectionModule", FACE_DETECTION_MODULE, graph_path) {
}

int FaceDetectionModule::PNet(cv::Mat& img, const ScaleWindow& win,
                   std::vector<FaceBox>* boxes) {
  std::unique_ptr<tensorflow::Session>& session = this->session();
	cv::Mat resized_img;
	int height = win.h;
	int width = win.w;
	float scale = win.scale;

	cv::resize(img, resized_img, cv::Size(width, height), 0, 0);

  Tensor input_tensor(DT_FLOAT, TensorShape({1, height, width, 3}));
  float* input_data_ptr = input_tensor.flat<float>().data();
  cv::Mat fake_mat(height, width, CV_32FC3, input_data_ptr);
  resized_img.convertTo(fake_mat, CV_32FC3);

  std::vector<std::pair<std::string, Tensor>> inputs;
  inputs.emplace_back(std::make_pair<std::string, Tensor>("pnet/input:0",
        std::move(input_tensor)));

  std::vector<std::string> output_names = {
    "pnet/conv4-2/BiasAdd:0",
    "pnet/prob1:0"
  };
  std::vector<Tensor> outputs;
  Status status = session->Run(inputs, output_names, {}, &outputs);

  if (!status.ok()) {
    LOG(WARNING) << "PNet failed:" << status.ToString();
		return -1;
	}

  const Tensor& regress_tensor  = outputs[0];
  const Tensor& confidence_tensor  = outputs[1];

  const TensorShape& regress_shape  = regress_tensor.shape();
	const TensorShape& confidence_shape = confidence_tensor.shape();

	int h = regress_shape.dim_size(1);
	int w = confidence_shape.dim_size(2);

  const tensorflow::StringPiece& regress_piece = regress_tensor.tensor_data();
  const tensorflow::StringPiece& confidence_piece = confidence_tensor.tensor_data();

  const float* regress_data = (const float*)regress_piece.data();
  const float* confidence_data = (const float*)confidence_piece.data();

  // int confidence_size = h * w * 2;

	std::vector<FaceBox> candidate_boxes;
	GenerateBoundingBox(confidence_data, regress_data, scale, kPNetThreshold,
                      h, w, true, &candidate_boxes);

	Normalize(candidate_boxes, 0.5, NMS_UNION, boxes);
  return 0;
}

int FaceDetectionModule::RNet(cv::Mat& img, std::vector<FaceBox>& pnet_boxes,
                  std::vector<FaceBox>* output_boxes) {
  std::unique_ptr<tensorflow::Session>& session = this->session();
	int batch = pnet_boxes.size();
	int height = 24;
	int width = 24;

	int input_size = batch * height * width * 3;
	std::vector<float> input_buffer(input_size);
	float* input_data = input_buffer.data();

  int patch_size = width * height * 3;
	for (int i = 0; i < batch; i++) {
		Patch(img, pnet_boxes[i], input_data, height, width);
		input_data += patch_size;
	}

  Tensor input_tensor(DT_FLOAT, TensorShape({batch, height, width, 3}));
  float* input_data_ptr = input_tensor.flat<float>().data();
  std::copy_n(input_buffer.begin(), input_size, input_data_ptr);

  std::vector<std::pair<std::string, Tensor>> inputs;
  inputs.emplace_back(std::make_pair<std::string, Tensor>("rnet/input:0",
        std::move(input_tensor)));

  std::vector<std::string> output_names = {
    "rnet/conv5-2/conv5-2:0",
    "rnet/prob1:0"
  };

  std::vector<Tensor> outputs;
  Status status = session->Run(inputs, output_names, {}, &outputs);

  if (!status.ok()) {
    LOG(WARNING) <<"RNet failed:" << status.ToString();
		return -1;
	}

  const Tensor& regress_tensor  = outputs[0];
  const Tensor& confidence_tensor  = outputs[1];
  const tensorflow::StringPiece& regress_piece = regress_tensor.tensor_data();
  const tensorflow::StringPiece& confidence_piece = confidence_tensor.tensor_data();
  const float* regress_data = (const float*)regress_piece.data();
  const float* confidence_data = (const float*)confidence_piece.data();

	for (int i = 0; i < batch; i++) {
		if (confidence_data[1] > kRNetThreshold) {
			FaceBox box;
			FaceBox& input_box = pnet_boxes[i];
			box.x0 = input_box.x0;
			box.y0 = input_box.y0;
			box.x1 = input_box.x1;
			box.y1 = input_box.y1;
			box.score = *(confidence_data + 1);
			box.regress[0] = regress_data[1];
			box.regress[1] = regress_data[0];
			box.regress[2] = regress_data[3];
			box.regress[3] = regress_data[2];
			output_boxes->emplace_back(box);
		}
		confidence_data += 2;
		regress_data += 4;
	}
  return 0;
}

int FaceDetectionModule::ONet(cv::Mat& img, std::vector<FaceBox>& rnet_boxes,
                   std::vector<FaceBox>* output_boxes) {
  std::unique_ptr<tensorflow::Session>& session = this->session();
	int batch = rnet_boxes.size();
	int height = 48;
	int width = 48;
	int input_size = batch * height * width * 3;

	std::vector<float> input_buffer(input_size);
	float* input_data = input_buffer.data();

	for (int i = 0; i < batch; i++) {
		int patch_size = width * height * 3;
		Patch(img, rnet_boxes[i], input_data, height, width);
		input_data += patch_size;
	}

  Tensor input_tensor(DT_FLOAT, TensorShape({batch, height, width, 3}));
  float* input_data_ptr = input_tensor.flat<float>().data();
  std::copy_n(input_buffer.begin(), input_size, input_data_ptr);

  std::vector<std::pair<std::string, Tensor>> inputs;
  inputs.emplace_back(std::make_pair<std::string, Tensor>("onet/input:0",
        std::move(input_tensor)));

  std::vector<std::string> output_names = {
    "onet/conv6-2/conv6-2:0",
    "onet/conv6-3/conv6-3:0",
    "onet/prob1:0"
  };

  std::vector<Tensor> outputs;
  Status status = session->Run(inputs, output_names, {}, &outputs);

  if (!status.ok()) {
    LOG(WARNING) <<"RNet failed:" << status.ToString();
		return -1;
	}

  const Tensor& regress_tensor  = outputs[0];
  const Tensor& points_tensor  = outputs[1];
  const Tensor& confidence_tensor  = outputs[2];
  const tensorflow::StringPiece& regress_piece = regress_tensor.tensor_data();
  const tensorflow::StringPiece& points_piece = points_tensor.tensor_data();
  const tensorflow::StringPiece& confidence_piece = confidence_tensor.tensor_data();
  const float* regress_data = (const float*)regress_piece.data();
  const float* points_data = (const float*)points_piece.data();
  const float* confidence_data = (const float*)confidence_piece.data();

	for (int i = 0; i < batch; i++) {
		if (confidence_data[1] > kOnetThreshold) {
			FaceBox box;
			FaceBox& input_box = rnet_boxes[i];
			box.x0 = input_box.x0;
			box.y0 = input_box.y0;
			box.x1 = input_box.x1;
			box.y1 = input_box.y1;
			box.score = confidence_data[1];
			box.regress[0] = regress_data[1];
			box.regress[1] = regress_data[0];
			box.regress[2] = regress_data[3];
			box.regress[3] = regress_data[2];
			for (int j = 0; j < 5; j++) {
				box.landmark.x[j] = *(points_data + j + 5);
				box.landmark.y[j] = *(points_data + j);
			}
			output_boxes->emplace_back(box);
		}
		confidence_data += 2;
		regress_data += 4;
		points_data += 10;
	}
  return 0;
}

util::Status FaceDetectionModule::Predict(const Request& request,
                                          Response* response) {
  const std::unique_ptr<rapidjson::Document>& req_json_doc = request.req_json_doc();
  assert(req_json_doc != nullptr);

  std::string image_path;
  if (req_json_doc->HasMember("input_image_path")
      && (*req_json_doc)["input_image_path"].IsString()) {
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

  const float kAlpha = 0.0078125;
  const float kMean = 127.5;
  const int kMinSize = 40;
  const float kFactor = 0.709;

	cv::Mat img;
	frame.convertTo(img, CV_32FC3);
	img = (img - kMean) * kAlpha;
	img = img.t();
	cv::cvtColor(img, img, cv::COLOR_BGR2RGB);

	int img_h = img.rows;
	int img_w = img.cols;
	std::vector<ScaleWindow> scale_windows;
	PyramidScales(img_h, img_w, kMinSize, kFactor, &scale_windows);

	std::vector<FaceBox> pnet_boxes;
	std::vector<FaceBox> pnet_boxes_final;
  for (const ScaleWindow& scale_window : scale_windows) {
		std::vector<FaceBox> boxes;
		if (PNet(img, scale_window, &boxes) != 0) {
      continue;
    }
		pnet_boxes.insert(pnet_boxes.end(), boxes.begin(), boxes.end());
	}
	ProcessBoxes(pnet_boxes, img_h, img_w, &pnet_boxes_final);

	std::vector<FaceBox> rnet_boxes;
	std::vector<FaceBox> rnet_boxes_final;
	if (RNet(img, pnet_boxes_final, &rnet_boxes) != 0) {
    LOG(ERROR) << "Face detection RNet error, rid: " << request.rid();
    return util::Status(error::FACE_DETECTION_RNET_ERROR,
                        "Face detection RNet error");
  }
	ProcessBoxes(rnet_boxes, img_h, img_w, &rnet_boxes_final);

  float h = 0.0f;
  float w = 0.0f;
	std::vector<FaceBox> onet_boxes;
	if (ONet(img, rnet_boxes_final, &onet_boxes) != 0) {
    LOG(ERROR) << "Face detection ONet error, rid: " << request.rid();
    return util::Status(error::FACE_DETECTION_ONET_ERROR,
                        "Face detection ONet error");
  }

  for (FaceBox& box : onet_boxes) {
		h = box.x1 - box.x0 + 1;
		w = box.y1 - box.y0 + 1;
		for (int j = 0; j < 5; j++) {
			box.landmark.x[j] = box.x0 + w * box.landmark.x[j] - 1;
			box.landmark.y[j] = box.y0 + h * box.landmark.y[j] - 1;
		}
	}

	Regress(&onet_boxes);
  std::vector<FaceBox> face_boxes;
	Normalize(onet_boxes, 0.7, NMS_MIN, &face_boxes);

  for (FaceBox& box : face_boxes) {
		std::swap(box.x0, box.y0);
		std::swap(box.x1 ,box.y1);

		for(int l = 0; l < 5; l++) {
			std::swap(box.landmark.x[l], box.landmark.y[l]);
		}
	}

  if (!face_boxes.empty()) {
    uint64_t now = util::TimeUtils::GetCurrentTime();
    std::string save_file_name = FLAGS_face_detection_res_path
      + request.rid() + "_" + std::to_string(now) + ".jpg";
    for (FaceBox& box : face_boxes) {
      DrawRectAndLandmark(frame, box);
    }
    cv::imwrite(save_file_name, frame);

    // response->AddMember("result", FLAGS_result_http_server + save_file_name);
    std::string res_image_url = FLAGS_result_http_server + save_file_name;
    response->AddMember("image", res_image_url);
    response->AddMember("status", "true");
    response->set_success(true);
  } else {
    response->AddMember("status", "false");
    response->set_success(false);
  }
  return util::Status::OK();
}

void FaceDetectionModule::PyramidScales(int height, int width, int min_size,
    float factor, std::vector<ScaleWindow>* scale_windows) {
	int min_side = std::min(height, width);
	double m = 12.0 / min_size;
	min_side = min_side * m;
	double cur_scale = 1.0;
	double scale;

	while (min_side >= 12) {
		scale = m * cur_scale;
		cur_scale = cur_scale * factor; 
		min_side *= factor;

		int hs = std::ceil(height * scale);
		int ws = std::ceil(width * scale);

    ScaleWindow s;
    s.h = hs;
    s.w = ws;
    s.scale = scale;
		scale_windows->emplace_back(s);
	}
}

void FaceDetectionModule::ProcessBoxes(std::vector<FaceBox>& input, int img_h,
    int img_w, std::vector<FaceBox>* res) {
	Normalize(input, 0.7, NMS_UNION, res); 
	Regress(res);
	Square(res);
	Padding(img_h, img_w, res);
} 

void FaceDetectionModule::Normalize(std::vector<FaceBox>& input,
    float threshold, int type, std::vector<FaceBox>* output) {
	std::sort(input.begin(), input.end(),
			[](const FaceBox& a, const FaceBox& b) {
			  return a.score > b.score;  
			});

	int box_num = input.size();
	std::vector<int> merged(box_num, 0);
  float area0 = 0.0f;
  float area1 = 0.0f;
  float score = 0.0f;

	for (int i = 0; i < box_num; i++) { 
		if (merged[i]) { continue; }

    const FaceBox& c = input[i];
		output->emplace_back(c);
		area0 = (c.y1 - c.y0 + 1) * (c.x1 - c.x0 + 1);

		for(int j = i + 1; j < box_num; j++) {
			if (merged[j]) { continue; }

      const FaceBox& n = input[j];

			float inner_x0 = std::max(c.x0, n.x0);
			float inner_y0 = std::max(c.y0, n.y0);
			float inner_x1 = std::min(c.x1, n.x1);
			float inner_y1 = std::min(c.y1, n.y1);
			float inner_h = inner_y1 - inner_y0 + 1;
			float inner_w = inner_x1 - inner_x0 + 1;
			if (inner_h <= 0 || inner_w <= 0) { continue; }

			float inner_area = inner_h * inner_w;
			area1 = (n.y1 - n.y0 + 1) * (n.x1 - n.x0 + 1);

			if (type == NMS_UNION) {
				score = inner_area / (area0 + area1 - inner_area);
      } else if (type == NMS_MIN) {
				score = inner_area / std::min(area0, area1);
			} else {
				score = 0.0f;
			}

			if (score > threshold) {
				merged[j] = 1;
      }
		}
	}
}

void FaceDetectionModule::Regress(std::vector<FaceBox>* boxes) {
  float h = 0.0f;
  float w = 0.0f;
  for (FaceBox& box : *boxes) {
		h = box.y1 - box.y0 + 1;
		w = box.x1 - box.x0 + 1;
		box.x0 = box.x0 + w * box.regress[0];
		box.y0 = box.y0 + h * box.regress[1];
		box.x1 = box.x1 + w * box.regress[2];
		box.y1 = box.y1 + h * box.regress[3];
	}    
}

void FaceDetectionModule::Square(std::vector<FaceBox>* boxes) {
  float h = 0.0f;
  float w = 0.0f;
  float l = 0.0f;
  for (FaceBox& box : *boxes) {
		h = box.y1 - box.y0 + 1;
		w = box.x1 - box.x0 + 1;
		l = std::max(h, w);
		box.x0 = box.x0 + (w - l) * 0.5;
		box.y0 = box.y0 + (h - l) * 0.5;
		box.x1 = box.x0 + l - 1;
		box.y1 = box.y0 + l - 1;
	}
}

void FaceDetectionModule::Padding(int img_h, int img_w, std::vector<FaceBox>* boxes) {
  for (FaceBox& box : *boxes) {
		box.px0 = std::max(box.x0, 1.0f);
		box.py0 = std::max(box.y0, 1.0f);
		box.px1 = std::min(box.x1, (float)img_w);
		box.py1 = std::min(box.y1, (float)img_h);
	}
} 

void FaceDetectionModule::Patch(const cv::Mat& img, FaceBox& box, float* data_to,
                        int height, int width) {
	cv::Mat resized(height, width, CV_32FC3, data_to);
	cv::Mat chop_img = img(cv::Range(box.py0, box.py1), cv::Range(box.px0, box.px1));
	int pad_top    = std::abs(box.py0 - box.y0);
	int pad_left   = std::abs(box.px0 - box.x0);
	int pad_bottom = std::abs(box.py1 - box.y1);
	int pad_right  = std::abs(box.px1 - box.x1);
	cv::copyMakeBorder(chop_img, chop_img, pad_top, pad_bottom, pad_left,
                     pad_right, cv::BORDER_CONSTANT, cv::Scalar(0));
	cv::resize(chop_img, resized, cv::Size(width, height), 0, 0);
}

void FaceDetectionModule::GenerateBoundingBox(const float* confidence_data,
    const float* regress_data, float scale, float threshold, int h, int w,
    bool transposed, std::vector<FaceBox>* output) {
  const	int kStride = 2;
  const	int kCellSize = 12;

  float score = 0.0f;
  float top_x = 0.0f;
  float top_y = 0.0f;
  float bottom_x = 0.0f;
  float bottom_y = 0.0f;
  int score_offset = 0;
  int regress_offset = 0;

	for (int y = 0; y < h; y++) {
    for (int x = 0; x < w; x++) {
      score_offset = 2 * w * y + 2 * x + 1;
			score = confidence_data[score_offset];
			if (score >= threshold) {
				top_x = (int)((x * kStride + 1) / scale);
				top_y = (int)((y * kStride + 1) / scale);
				bottom_x = (int)((x * kStride + kCellSize) / scale);
				bottom_y = (int)((y * kStride + kCellSize) / scale);

				FaceBox box;
				box.x0 = top_x;
				box.y0 = top_y;
				box.x1 = bottom_x;
				box.y1 = bottom_y;
				box.score = score;

				regress_offset = (w * 4) * y + 4 * x;
				if (transposed) {
					box.regress[1] = regress_data[regress_offset];
					box.regress[0] = regress_data[regress_offset + 1]; 
					box.regress[3] = regress_data[regress_offset + 2];
					box.regress[2] = regress_data[regress_offset + 3];
				} else {
					box.regress[0] = regress_data[regress_offset];
					box.regress[1] = regress_data[regress_offset + 1]; 
					box.regress[2] = regress_data[regress_offset + 2];
					box.regress[3] = regress_data[regress_offset + 3];
				}
				output->emplace_back(box);
			}
		}
  }
}

// Draw landmarks and rectangle arround the face
void FaceDetectionModule::DrawRectAndLandmark(cv::Mat& frame, FaceBox& box) {
  cv::rectangle(frame, cv::Point(box.x0, box.y0), cv::Point(box.x1, box.y1),
                cv::Scalar(0, 255, 0), 1);

  for (int l = 0; l < 5; l++) {
		cv::circle(frame, cv::Point(box.landmark.x[l], box.landmark.y[l]), 1,
               cv::Scalar(0, 0, 255), 2);
	}
}

// void FaceDetectionModule::SaveImage(std::string file_name, cv::Mat& frame) {
  // cv::imwrite(file_name, frame);
// }

#if 0
// Init tensorflow session and create graph
int FaceDetectionModule::Init(const std::string& model_file) {
  // Initialize tensorflow session
  Status status = NewSession(SessionOptions(), &session_);
  if (!status.ok()) {
    LOG(FATAL) << "Init tensorflow session failed:" << status.ToString();
    return -1;
  } else {
		LOG(INFO) << "Init tensorflow session successfully";
  }

  // Load graph
  GraphDef graph_def;
  status = ReadBinaryProto(Env::Default(), model_file, &graph_def);
  if (!status.ok()) {
    LOG(FATAL) << "Load model " << model_file << " failed:" << status.ToString();
		return -1;
	} else {
		LOG(INFO) << "Load model " << model_file << " successfully";
  }

  // Add graph to the session
  status = session_->Create(graph_def);
  if (!status.ok()) {
    LOG(FATAL) << "Create graph in the session failed:" << status.ToString();
		return -1;
	} else {
		LOG(INFO) << "Create graph in the session successfully";
  }
  return 0;
}
#endif

}  // namespace prediction

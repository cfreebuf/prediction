// CopyRight 2019 360. All rights reserved.
// File   common_gflags.cpp
// Date   2019-10-28 17:30:09
// Brief

#include "common/common_gflags.h"

namespace prediction {

// Server
DEFINE_string(ip, "0.0.0.0", "Server ip or hostname to bind to");
DEFINE_int32(http_port, 10000, "Port to listen on with HTTP protocol");
DEFINE_int32(spdy_port, 10001, "Port to listen on with SPDY protocol");
DEFINE_int32(http2_port, 10002, "Port to listen on with HTTP2 protocol");
DEFINE_int32(server_threads, 0, "Number of threads to listen on. "
            "<=0 will use the number of cores on this machine");

// Prediction
DEFINE_string(prediction_conf_file, "./conf/prediction.conf", "");
DEFINE_int32(reload_prediction_interval, 10, "in seconds");

// Experiment
DEFINE_string(exp_conf_path, "./conf/exp", "");
DEFINE_int32(reload_exp_interval, 10, "in seconds");
DEFINE_string(chain_conf, "./conf/chains", "");

// Monitor
DEFINE_string(monitor_status, "./data/monitor_status", "monitor status");
DEFINE_string(monitor_data, "./data/monitor.dat", "monitor data");
DEFINE_int32(monitor_interval, 60, "monitor interval");

// Reloader
DEFINE_int32(reloader_interval, 10, "reload interval in seconds");
DEFINE_string(redis_host, "127.0.0.1", "redis ip or redis cluster addr");
DEFINE_int32(redis_port, 6379, "redis port, not used for redids cluster");

// GRPC client
DEFINE_int32(client_connection_timeout, 50, "timeout for grpc clinet in milliseconds");

// ThreadPool
DEFINE_int32(worker_thread_count, 100, "count of prediction threads");
DEFINE_int32(sample_count_per_task, 100, "count of samples in one task");
DEFINE_bool(enable_predict_parallel, true, "");

// tensorflow serving
DEFINE_string(tensorflow_serving_addr, "localhost:9000", "tensorflow serving addr list");
DEFINE_string(tensorflow_serving_name, "tensorflow_model_server", "tensorflow serving consul server name");
DEFINE_int32(tensorflow_serving_conn_retry_times, 1, "tensorflow serving retry times");
DEFINE_int32(tensorflow_serving_conn_retry_interval_ms, 0, "tensorflow serving retry interval ms");
DEFINE_int32(tensorflow_serving_min_conn, 10, "tensorflow serving retry interval ms");
DEFINE_int32(tensorflow_serving_max_conn, 50, "tensorflow serving retry interval ms");
DEFINE_int32(tensorflow_serving_conn_rebalance_second ,300 , "tensorflow server connect rebalance secod");

// Models
// MTCNN Model
DEFINE_string(mtcnn_model, "./data/models/mtcnn/mtcnn_frozen_model.pb", "mtcnn model");
// Logo Model
DEFINE_string(logo_model, "./data/models/logo/logo_frozen_model.pb", "logo model");

DEFINE_string(result_http_server, "http://localhost:8086/", "http server to get images after processed");

DEFINE_string(face_detection_req_path, "./data/face_detection/request/", "request path to store images");
DEFINE_string(face_detection_res_path, "./data/face_detection/response/", "response path to store images");

DEFINE_string(logo_detection_req_path, "./data/logo_detection/request/", "request path to store images");
DEFINE_string(logo_detection_res_path, "./data/logo_detection/response/", "response path to store images");

// Test
DEFINE_string(face_detection_req_image_url, "", "response path to store images");
DEFINE_string(logo_detection_req_image_url, "", "response path to store images");

// Debug
DEFINE_bool(debug, false, "");

}  // namespace prediction

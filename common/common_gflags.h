// CopyRight 2019 360. All rights reserved.
// File   common_gflags.h
// Date   2019-10-28 17:40:26
// Brief

// File   common_gflags.h
// Author lidongming
// Date   2018-08-28 11:46:18
// Brief

#ifndef PREDICTION_COMMON_COMMON_GFLAGS_H_
#define PREDICTION_COMMON_COMMON_GFLAGS_H_

#include "gflags/gflags.h"

namespace prediction {

// Server
DECLARE_string(ip);
DECLARE_int32(http_port);
DECLARE_int32(spdy_port);
DECLARE_int32(http2_port);
DECLARE_int32(server_threads);

// Prediction
DECLARE_string(prediction_conf_file);
DECLARE_int32(reload_prediction_interval);

// Experiment
DECLARE_string(exp_conf_path);
DECLARE_int32(reload_exp_interval);
DECLARE_string(chain_conf);

// Monitor
DECLARE_string(monitor_status);
DECLARE_string(monitor_data);
DECLARE_int32(monitor_interval);

// Reloader
DECLARE_int32(reloader_interval);
DECLARE_string(redis_host);
DECLARE_int32(redis_port);

// GRPC client
DECLARE_int32(client_connection_timeout);

// ThreadPool
DECLARE_int32(worker_thread_count);
DECLARE_int32(sample_count_per_task);
DECLARE_bool(enable_predict_parallel);

DECLARE_string(tensorflow_serving_addr);
DECLARE_string(tensorflow_serving_name);
DECLARE_int32(tensorflow_serving_conn_retry_times);
DECLARE_int32(tensorflow_serving_conn_retry_interval_ms);
DECLARE_int32(tensorflow_serving_min_conn);
DECLARE_int32(tensorflow_serving_max_conn);
DECLARE_int32(tensorflow_serving_conn_rebalance_second);

// Models
// MTCNN model
DECLARE_string(mtcnn_model);
// logo model
DECLARE_string(logo_model);

DECLARE_string(result_http_server);
DECLARE_string(face_detection_req_path);
DECLARE_string(face_detection_res_path);
DECLARE_string(logo_detection_req_path);
DECLARE_string(logo_detection_res_path);

// Test
DECLARE_string(face_detection_req_image_url);
DECLARE_string(logo_detection_req_image_url);

// Debug
DECLARE_bool(debug);

}  // namespace prediction

#endif  // PREDICTION_COMMON_COMMON_GFLAGS_H_

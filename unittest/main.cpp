// CopyRight 2019 360. All rights reserved.
// File   main.cpp
// Date   2019-10-29 15:25:10
// Brief

#include "prediction/third_party/gtest/include/gtest/gtest.h"
// #include "prediction/third_party/glog/include/glog/logging.h"
#include "prediction/common/common_gflags.h"
#include "prediction/util/logging.h"

using namespace google;
int main(int argc, char **argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  gflags::SetCommandLineOption("flagfile", "./conf/test_gflags.conf");
  testing::InitGoogleTest(&argc, argv);
  google::InitGoogleLogging(argv[0]);
  google::FlushLogFiles(google::INFO);
  FLAGS_logtostderr = 1;
  FLAGS_log_dir = "./logs";
  return RUN_ALL_TESTS();
}

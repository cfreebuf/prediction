# 在线实时预估平台

## 主要目标
基于tensoflow框架及生成的模型，支持多种AI相关的业务场景，提供统一的平台，支持模型的快速迭代，用c++实现，尽可能的提高性能

## 主要功能
1、人脸检测，采用mtcnn tensorflow模型
2、Logo检测，参考：https://github.com/ShubhamAvasthi/logo-detector
TODO

## 主要模块
### 集成proxygen
采用facebook的proxygen作为webserver，提供restful接口

### 采用tensoflow api加载模型及predict
使用tensorflow的api进行模型的加载与预测

### 多层实验框架
多层实验框架用于流量的多层分配及模型快速迭代

### 动态可配的链式Processor
基于配置的链式processor处理流程，用于业务处理的动态可配

## 框架及数据流程
TODO

## 编译步骤
### 编译tensorflow
#### Build include
bazel build //tensorflow:install_headers

#### Build shared libs
生成libtensorflow_cc.so和libtensorflow_framework.so
bazel build //tensorflow:libtensorflow_cc.so

#### copy .so
bazel-bin/tensorflow/libtensorflow_cc.so
bazel-bin/tensorflow/libtensorflow_framework.so

### 编译tensorflow serving
#### Build libs and bin
bazel build tensorflow_serving/model_servers:tensorflow_model_server
#### copy apis
bazel-bin/tensorflow_serving/apis/

### 编译proxygen
proxygen的编译较为复杂，后续补充编译流程及问题记录

### 编译其它依赖模块
grpc、protobuf、gflags、glog等

### 依赖的相关库及版本
#### tensorflow
version: 1.15.0
source:  https://github.com/tensorflow/tensorflow/archive/v1.15.0.tar.gz
commit:  590d6eef7e91a6a7392c8ffffb7b58f2e0c8bc6b

#### tensorflow-serving
version: 1.15.0
source:  https://github.com/tensorflow/serving/archive/1.15.0.tar.gz
commit:  748217e48b006cb0d8d87d362bce88cd5f292a73

#### protobuf
version: 3.8.0
source:  https://github.com/protocolbuffers/protobuf/archive/v3.8.0.tar.gz
commit:  09745575a923640154bcf307fba8aedff47f240a

#### gtest
version: 1.10.0
source:  https://github.com/google/googletest/archive/release-1.10.0.tar.gz
commit:  703bd9caab50b139428cea1aaff9974ebee5742e

#### glog
version: 0.4.0
source:  https://github.com/google/glog/archive/v0.4.0.tar.gz
commit:  96a2f23dca4cc7180821ca5f32e526314395d26a

#### gflags
version: 2.2.2
source:  https://github.com/gflags/gflags/archive/v2.2.2.tar.gz
commit:  e171aa2d15ed9eb17054558e0b3a6a413bb01067

#### jemalloc
version: 5.2.1
source:  https://github.com/jemalloc/jemalloc/archive/5.2.1.tar.gz
commit:  ea6b3e973b477b8061e0076bb257dbd7f3faa756

#### bazel
version: 0.26.0
source:  https://github.com/bazelbuild/bazel/releases/download/0.26.0/bazel-0.26.0-installer-linux-x86_64.sh

#### c-ares
version: 0.15.0
source:  https://c-ares.haxx.se/download/c-ares-1.15.0.tar.gz

#### grpc
version: 1.25.0-pre1
source:  https://github.com/grpc/grpc/archive/v1.25.0-pre1.tar.gz

#### gperftools
version: 2.7
source:  https://github.com/gperftools/gperftools/archive/gperftools-2.7.tar.gz

#### libconfig
version: 1.7.2
source:  https://github.com/hyperrealm/libconfig/archive/v1.7.2.tar.gz

#### zstd
version: 1.4.3
source:  https://github.com/facebook/zstd/releases/tag/v1.4.3

#### gcc/g++
version: 9.2.0

## 后续计划
1、完善支持请求tensorflow_serving的相关逻辑
2、支持通过rpc请求python服务
3、补充编译步骤
4、支持更多的AI服务

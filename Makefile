ifeq ($(shell uname -m),x86_64)

CC=gcc
CXX=g++

CCHECK=../tools/cpplint.py

# Modules
MODULE=prediction

CXXFLAGS += -O3 -DNDEBUG
CXXFLAGS +=-g \
		 -pipe \
		 -W \
		 -Wall \
		 -m64 \
		 -Wextra \
		 -std=c++14 \
		 -ffast-math \
		 -Wno-unused-parameter \
		 -Wno-unused-variable \
		 -Wno-unused-function \
		 -Wno-aligned-new \
		 -Wno-return-type \
		 -Wno-array-bounds \
		 -Wno-sign-compare \
		 -Wno-implicit-fallthrough \
		 -Wno-sizeof-pointer-div \
		 -DGTEST_HAS_TR1_TUPLE=0 -DGTEST_USE_OWN_TR1_TUPLE=0 \
		 -D_GLIBCXX_USE_CXX11_ABI=0

LDFLAGS= -L./libs -ltensorflow_cc -ltensorflow_framework \
				 -lcurl -lsodium -levent -lunwind \
				 -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_ml \
				 -lopencv_videoio -lopencv_imgcodecs \
				 -Wl,-rpath=.:..:./libs:../libs

INC_DIR += -I. -I.. -I./server \
	  -Ithird_party \
		-Ithird_party/curl/include \
		-Ithird_party/c-ares/include \
		-Ithird_party/grpc/include \
		-Ithird_party/hiredis/include \
		-Ithird_party/libconfig/include \
		-Ithird_party/rapidjson \
		-Ithird_party/cityhash/include \
		-Ithird_party/gflags/include \
		-Ithird_party/glog/include \
		-Ithird_party/gtest/include \
		-Ithird_party/protobuf/include \
		-Ithird_party/tensorflow/include \
		-Ithird_party/tensorflow_serving/include \
		-Ithird_party/proxygen/include \
		-Ithird_party/opencv/include

BOOST_LIBS=/usr/local/lib/libboost_context.a
LIBS= -Xlinker "-(" \
	  $(BOOST_LIBS) \
	  third_party/c-ares/lib/libcares.a \
	  third_party/cityhash/lib/libcityhash.a \
		third_party/hiredis/lib/libhiredis.a \
		third_party/libconfig/lib/libconfig.a \
		third_party/libconfig/lib/libconfig++.a \
		third_party/grpc/lib/libgrpc.a \
		third_party/grpc/lib/libgrpc++.a \
		third_party/protobuf/lib/libprotobuf.a \
		third_party/protobuf/lib/libprotobuf-lite.a \
		third_party/glog/lib/libglog.a \
		third_party/gtest/lib/libgtest.a \
		third_party/gflags/lib/libgflags.a \
		third_party/proxygen/lib/libfizz.a \
		third_party/proxygen/lib/libfolly.a \
		third_party/proxygen/lib/libwangle.a \
		third_party/proxygen/lib/libproxygen.a \
		third_party/proxygen/lib/libproxygenhttpserver.a \
	  third_party/tensorflow_serving/lib/libtensorflow_serving.a \
		-pthread -ldl -lz -lrt -lssl -lm -lc -lcrypto \
		-Xlinker "-)"

		# -static-libgcc -static-libstdc++ 
ifdef PRINT_BACKSTRACE
CXXFLAGS += -rdynamic -DPRINT_BACKSTRACE
endif

# TCMALLOC="enable"
ifdef TCMALLOC
LDFLAGS += -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
CXXFLAGS += -DUSE_TCMALLOC
INC_DIR += -Ithird_party/gperftools/include
LIBS += third_party/gperftools/lib/libtcmalloc_and_profiler.a
endif

ifdef JEMALLOC
LDFLAGS += -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
CXXFLAGS += -DUSE_JEMALLOC
INC_DIR += -Ithird_party/jemalloc/include
LIBS += third_party/jemalloc/lib/libjemalloc.a
endif

all: PRE_BUILD bin/$(MODULE) unittest/test
	@echo "[[32mBUILD[0m][Target:'[32mall[0m']"
	@cp -rf bin output/
	@cp -rf conf output/
	@cp -rf shell output/
	@cp -rf data output/
	@cp -rf libs output/
	@echo "[[32mmake all done[0m]"

system-check:
	@echo "[[32mCHECK DEPENDENCY[0m]"

# 语法规范检查
style:
	python ../tools/cpplint.py --extensions=hpp,cpp --linelength=80 *.cpp

clean:
	@find . -name "*.o" | xargs -I {} rm {}
	@rm -rf bin
	@rm -rf logs
	@rm -rf unittest/test
	@rm -rf output
.phony:clean

PRE_BUILD:
	@mkdir -p bin
	@mkdir -p logs
	@mkdir -p output/logs

IGNORE_FILE_PATERN="main/main.cc|client/main.cpp|unittest/main.cpp"
SERVER_OBJS += $(patsubst %.cpp,%.o, $(shell find . -type f -name *.cpp | egrep -v $(IGNORE_FILE_PATERN)))
SERVER_OBJS += $(patsubst %.cc, %.o, $(shell find . -type f -name *.cc  | egrep -v $(IGNORE_FILE_PATERN)))
SERVER_OBJS += $(patsubst %.c,  %.o, $(shell find . -type f -name *.c   | egrep -v $(IGNORE_FILE_PATERN)))

%.o:%.cpp
	@echo "[[32mBUILD[0m][Target:'[32m$<[0m']"
	$(CXX) $(CXXFLAGS) $(INC_DIR) -c $< -o $@

%.o:%.cc
	@echo "[[32mBUILD[0m][Target:'[32m$<[0m']"
	@$(CXX) $(CXXFLAGS) $(INC_DIR) -c $< -o $@

%.o:%.c
	@echo "[[32mBUILD[0m][Target:'[32m$<[0m']"
	@$(CXX) $(CXXFLAGS) $(INC_DIR) -c $< -o $@

bin/$(MODULE) : $(SERVER_OBJS) 
	$(CXX) -o $@ $(INC_DIR) $(LDFLAGS) $(CXXFLAGS) \
		main/main.cc $(BOOST_LIBS) $(SERVER_OBJS) $(LIBS)

unittest/test: $(SERVER_OBJS) 
	$(CXX) -o $@ $(INC_DIR) $(LDFLAGS) $(CXXFLAGS) \
		unittest/main.cpp $(BOOST_LIBS) $(SERVER_OBJS) $(LIBS)

endif #ifeq ($(shell uname -m),x86_64)

CC=/usr/local/bin/gcc
CXX=/usr/local/bin/g++

CXXFLAGS=
LDFLAGS= -pthread

LIBS=
INCLUDES= -I. -I/home/lidongming/local/eigen/include
INCLUDES+= -I/home/lidongming/local/eigen/include/eigen3

IGNORE_FILE_PATERN="main.cpp|main.cc|back"
OBJS=$(patsubst %.cc,   %.o, $(shell find . -type f -name '*.cc'  | egrep -v $(IGNORE_FILE_PATERN)))
OBJS+=$(patsubst %.cpp, %.o, $(shell find . -type f -name '*.cpp' | egrep -v $(IGNORE_FILE_PATERN)))

all:threadpool
	echo "make all done"

%.o:%.cc
	echo "BUILD TARGET:'$<'"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

%.o:%.cpp
	echo "BUILD TARGET:'$<'"
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

threadpool:$(OBJS)
	$(CXX) -o $@ $(INCLUDES) $(LDFLAGS) $(CXXFLAGS) \
		main.cpp $(LIBS) $(OBJS)

clean:
	find . -name "*.o" | xargs -I {} rm {}
	rm threadpool
.phony:clean

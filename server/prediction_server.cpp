// CopyRight 2019 360. All rights reserved.
// File   prediction.cpp
// Date   2019-10-31 10:08:07
// Brief

#include <boost/shared_ptr.hpp>
#include <folly/io/async/EventBaseManager.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/Unistd.h>
#include <proxygen/httpserver/HTTPServer.h>

#include "prediction/server/prediction_server.h"
#include "prediction/util/logging.h"
#include "prediction/common/common_gflags.h"
#include "prediction/util/time_utils.h"
#include "prediction/util/monitor.h"
#include "prediction/util/logging.h"
#include "prediction/util/threadpool/threadpool.h"
#include "prediction/server/prediction_handler.h"
#include "prediction/server/tensorflow_serving_adapter.h"

using namespace proxygen;
using folly::SocketAddress;
using Protocol = HTTPServer::Protocol;

namespace prediction {

void PredictionServer::Start() {
  std::vector<HTTPServer::IPConfig> addrs = {
    {SocketAddress(FLAGS_ip, FLAGS_http_port, true), Protocol::HTTP},
    {SocketAddress(FLAGS_ip, FLAGS_spdy_port, true), Protocol::SPDY},
    {SocketAddress(FLAGS_ip, FLAGS_http2_port, true), Protocol::HTTP2},
  };

  if (FLAGS_server_threads <= 0) {
    FLAGS_server_threads = sysconf(_SC_NPROCESSORS_ONLN);
    CHECK(FLAGS_server_threads > 0);
  }

  LOG(INFO) << "threads: " << FLAGS_server_threads;

  HTTPServerOptions options;
  options.threads = static_cast<size_t>(FLAGS_server_threads);
  options.idleTimeout = std::chrono::milliseconds(60000);
  options.shutdownOn = {SIGINT, SIGTERM};
  options.enableContentCompression = false;
  options.handlerFactories = RequestHandlerChain()
      .addThen<PredictionHandlerFactory>()
      .build();
  // Increase the default flow control to 1MB/10MB
  options.initialReceiveWindow = uint32_t(1 << 20);
  options.receiveStreamWindowSize = uint32_t(1 << 20);
  options.receiveSessionWindowSize = 10 * (1 << 20);
  options.h2cEnabled = true;

  HTTPServer server(std::move(options));
  // LOG(INFO) << "listen on: " << addrs[0];
  server.bind(addrs);

  // Start HTTPServer mainloop in a separate thread
  LOG(INFO) << "Start HTTPServer mainloop...";
  std::thread t([&] () {
    server.start();
  });

  t.join();
}

}  // namespace prediction

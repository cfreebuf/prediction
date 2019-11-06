// CopyRight 2019 360. All rights reserved.
// File   chain_handler.cpp
// Date   2019-10-29 11:17:29
// Brief

#include "prediction/chain/chain_handler.h"
#include "prediction/chain/chain_factory.h"
#include "prediction/chain/factory_context.h"
#include "prediction/processor/processor_factory.h"
#include "prediction/renderer/renderer_factory.h"
#include "prediction/chain/execution.h"
#include "prediction/server/result.h"
#include "prediction/util/time_utils.h"
#include "util/logging.h"

namespace prediction {

using namespace util;

ChainHandler::ChainHandler(ChainFactory* chain_factory)
  : chain_factory_(chain_factory) {
}

ChainHandler::~ChainHandler() { }

int ChainHandler::RegisterRequestProcessor(RequestProcessor* p) {
  bool found = false;
  for (auto& processor : request_processor_) {
    if (processor == p) {
      found = true;
      break;
    }
  }

  if (found) { return -1; }
  else { request_processor_.push_back(p); }

  return 0;
}

int ChainHandler::UnRegisterRequestProcessor(RequestProcessor* p) {
  bool found = false;
  for (auto it = request_processor_.begin();
      it != request_processor_.end(); ++it) {
    if (*it == p) {
      found = true;
      request_processor_.erase(it);
      break;
    }
  }

  if (found) { return 0; }
  else { return -1; }
}

int ChainHandler::Handle(Request* req, Result* res) {
  if (!chain_factory_) return -1;

  for (const auto& it : request_processor_) {
    it->Process(req);
  }

  std::string chain_id = req->Get("chain");

  const Chain* chain = chain_factory_->GetChain(chain_id);
  if (!chain) {
    LOG(WARNING) << "not found chain:" << chain_id << " rid:" << req->rid();
    return -1;
  }
  // LOG(INFO) << "found chain:" << chain_id << " rid:" << req->rid();

  // Init execution
  Execution execution(chain);

  // Init context
  Context context(req, res, &execution);

  static const std::string PROCESS = "process";

  TimeRecorder timer;
  timer.StartTimer(PROCESS);
  // execute all processors one by one
  execution.Execute(context, NULL);
  timer.StopTimer(PROCESS);

#if 0
  long total_process_time = timer.GetElapse(PROCESS);
  context.SetDoubleProperty(PROCESS, total_process_time);
#endif

#if 0
  std::stringstream ss;
  std::string time_str = "total=" + std::to_string(total_process_time) + ",";
  std::string processors = "";
  // time_str += " {";
  for (auto it = processor_timer.begin(); it != processor_timer.end(); ++it) {
    time_str += it->first + "=" + std::to_string(it->second) + ",";
    processors += it->first + ",";
  }
  time_str.erase(time_str.end() - 1);
  if (!processors.empty()) {
    processors.erase(processors.end() - 1);
  }
  // time_str += "}";
  req->Set("PROCESS_TIME", time_str);
  req->Set("PROCESSORS", processors);
#endif

  // Now all processors executed, start rendering
  // Try to get renderer from request
  std::string renderer_name = req->Get("renderer");
  Renderer* renderer = NULL;
  if (!renderer_name.empty()) {
    // renderer = RenderFactory::GetRenderer(renderer_name);
    FactoryContext* factory_context = chain_factory_->factory_context();
    renderer = factory_context->renderer_factory->GetRenderer(renderer_name);
  }

  // rendering
  if (renderer != NULL) {
    renderer->Render(context);
  }

  return 0;
}

}

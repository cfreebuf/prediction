#ifndef MULTI_LAYER_EXP_HANDLER_H_
#define MULTI_LAYER_EXP_HANDLER_H_

#include "multi_layer_exp_framework.h"
#include "server/request.h"

namespace prediction {

static const std::string CONF_FILE_PATH = "./conf/exp";

class MultiLayerExpHandler {
 public:
  MultiLayerExpHandler();
  virtual ~MultiLayerExpHandler();

  int Init();
  int Reload(const std::string & path);
  int DoExp(Request& request);
  bool GetInitStatus();

 private:
  bool _init;
  MultiLayerExpFramework _framework;
};

class RequestContextImpl: public RequestContext {
 public:
  RequestContextImpl(Request& request);
  virtual ~RequestContextImpl();

  virtual std::string Get(const std::string & key, const std::string & default_str = "") const;
  virtual std::string aid() const;
  virtual const std::vector<std::string>& GetForceExpIds() const;
  virtual std::string rid() { return _request.rid(); }

 private:
  std::string _aid;
  std::vector<std::string> _force_expids;
  Request& _request;
};

}

#endif

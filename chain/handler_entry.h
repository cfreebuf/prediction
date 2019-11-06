// File   handler_entry.h
// Author lidongming
// Date   2018-09-26 11:39:21
// Brief

#ifndef PREDICTION_SERVER_CHAIN_HANDLER_ENTRY_H_
#define PREDICTION_SERVER_CHAIN_HANDLER_ENTRY_H_

#include <vector>
#include <memory>

#include "chain/base_handler.h"

namespace prediction {

class HandlerEntry : public BaseHandler {
 public:
  HandlerEntry();
  virtual ~HandlerEntry();
  virtual int Handle(Request* req, Result* res);

  // int RegisterHandler(const std::string& url_pattern, std::shared_ptr<BaseHandler>);
  // int UnRegisterHandler(const std::string& url_pattern);

  void set_default_handler(BaseHandler* handler);

 private:
#if 0
  struct HandlerMappingItem {
    std::string handler_pattern;
    std::shared_ptr<Regex> handler_regex;
    BaseHandler* handler;
  };
  bool ExistHandler(const std::string& pattern);
  std::vector<HandlerMappingItem> handler_mapping_;
#endif

 private:
  BaseHandler* default_handler_;
};  // HandlerEntry

}  // namespace prediction

#endif  // PREDICTION_SERVER_CHAIN_HANDLER_ENTRY_H_

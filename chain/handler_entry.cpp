// File   handler_entry.cpp
// Author lidongming
// Date   2018-09-26 11:37:46
// Brief

#include "chain/handler_entry.h"
#include "util/logging.h"

namespace prediction {

HandlerEntry::HandlerEntry() { }
HandlerEntry::~HandlerEntry() { }

void HandlerEntry::set_default_handler(BaseHandler* handler) {
  default_handler_ = handler;
}

int HandlerEntry::Handle(Request* req, Result* res) {
  // FIXME(lidongming):match handlers
  // BaseHandler* handler = NULL;
  // if (!handler) { handler = _default_handler; }

  BaseHandler* handler = default_handler_;

  handler->Handle(req, res);
  return 0;
}

#if 0
int HandlerEntry::RegisterHandler(const std::string& handler_pattern,
    std::shared_ptr<BaseHandler> handler) {
  if (ExistHandler(handler_pattern)) {
    LOG(FATAL) << "handler already exists handler:" << handler_pattern;
    return -1;
  } else {
    std::shared_ptr<Regex> regex(new Regex(handler_pattern));
    HandlerMappingItem item;
    item.handler_pattern = handler_pattern;
    item.handler_regex = regex;
    item.handler = handler;
    _handler_mapping.push_back(item);
  }
  return 0;
}

int HandlerEntry::UnRegisterHandler(const std::string& handler_pattern) {
  int ret = -1;
  auto it = _handler_mapping.begin();
  for (; it != _handler_mapping.end(); ++it) {
    if (it->handler_pattern == handler_pattern) {
      ret = 0;
      _handler_mapping.erase(it);
      break;
    }
  }
  return ret;
}

bool HandlerEntry::ExistHandler(const std::string& handler_pattern) {
  bool exists = false;
  auto it = _handler_mapping.begin();
  for (; it != _handler_mapping.end(); ++it) {
    if (it->handler_pattern == handler_pattern) {
      exists = true;
      break;
    }
  }
  return exists;
}
#endif

}

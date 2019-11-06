#ifndef EXP_COMMON_H_
#define EXP_COMMON_H_

#include <string>

namespace prediction {

enum {
    EXP_OK = 0,
    EXP_ERROR = -1,
    EXP_SKIP = 1,
};

enum DiversionType {
    DIV_UNKNOWN = -1,
    DIV_UID = 1,
    DIV_RANDOM = 2,
    DIV_TAIL_NUMBER = 3,
};

enum LoadConfigStatus {
    LOAD_SUCCESS = 0,
    LOAD_INVALID_FORMAT = -1,
};

class DiversionTypeUtils {
 public:
  static DiversionType from_string(const std::string& type) {
      if (type == "UID") { return DIV_UID; }
      else if (type == "RANDOM") { return DIV_RANDOM; }
      else if (type == "TAIL_NUMBER") { return DIV_TAIL_NUMBER; }

      return DIV_UNKNOWN;
  }
};

}

#endif /* EXP_COMMON_H_ */

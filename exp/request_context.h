#ifndef REQUEST_CONTEXT_H_
#define REQUEST_CONTEXT_H_

#include <string>
#include <map>
#include <vector>

namespace prediction {

class RequestContext {
 public:
  RequestContext();
  virtual ~RequestContext();
  virtual std::string Get(const std::string& key,
          const std::string& default_str="") const = 0;
  virtual std::string aid() const = 0;
  virtual const std::vector<std::string>& GetForceExpIds() const = 0;

 public:
  static int get_aid_hash(const std::string & aid, int hash_base);
};

} /* namespace prediction */

#endif /* REQUEST_CONTEXT_H_ */


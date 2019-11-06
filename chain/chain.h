// File   chain.h
// Author lidongming
// Date   2018-09-26 12:05:06
// Brief

#ifndef PREDICTION_SERVER_CHAIN_CHAIN_H_
#define PREDICTION_SERVER_CHAIN_CHAIN_H_

#include <map>
#include <vector>
#include <string>
#include "processor/processor.h"
#include "renderer/renderer.h"
#include "chain/variable.h"

namespace prediction {

class Chain {
 public:
  Chain(const std::vector<Processor*>& p, Renderer* renderer = NULL) {
    std::copy(p.begin() ,p.end(), std::back_inserter(processors_));
    renderer_ = renderer;
  }

  Chain() : renderer_(NULL) { }

  void CopyChain(const Chain& chain) {
    std::copy(chain.processors_.begin(), chain.processors_.end(),
        std::back_inserter(processors_));
    std::copy(chain.processor_params_.begin(),
        chain.processor_params_.end(),
        std::inserter(processor_params_, processor_params_.end()));
    if (chain.sub_chains_.size() > 0) {
      std::copy(chain.sub_chains_.begin(), chain.sub_chains_.end(),
          std::back_inserter(sub_chains_));
    }
    renderer_ = chain.renderer_;
  }

  Chain(const Chain& chain) {
    CopyChain(chain);
  }

  Chain& operator = (const Chain& chain) {
    Clear();
    CopyChain(chain);
    return *this;
  }

  void set_renderer(Renderer* renderer) { renderer_ = renderer; }

  Renderer* renderer() const { return renderer_; }

  void Clear() {
    processors_.clear();
    processor_params_.clear();
    sub_chains_.clear();
  }

  void AddProcessor(Processor* p) {
    processors_.push_back(p);
  }

  void SetProcessorParams(const std::map<std::string, Variable>& params) {
    processor_params_.clear();
    for (auto it = params.begin(); it != params.end(); ++it) {
      processor_params_[it->first] = it->second;
    }
  }

  const Variable& GetProcessorParam(std::string name){
    if (processor_params_.count(name) > 0){
      return processor_params_[name];
    }
    return Variable::NULLVAR; 
  }

  const std::map<std::string, Variable>& processor_params() {
    return processor_params_;
  }

  void SetSubChains(std::vector<Chain*>& sub_chains) {
    sub_chains_.clear();
    std::copy(sub_chains.begin(), sub_chains.end(),
        std::back_inserter(sub_chains_));
  }

  Chain* GetSubChain(std::string name) {
    auto it = sub_chains_.begin();
    for (;it != sub_chains_.end(); ++it) {
      if ((*it)->chain_name() == name) { return *it; }
    }
    return NULL;
  }

  Processor* GetNext(Processor* prev) {
    if (prev == NULL) {
      return processors_.empty() ? NULL : processors_[0];
    }
    for (auto it = processors_.begin(); it != processors_.end(); ++it) {
      if (*it == prev) {
        ++it;
        if (it != processors_.end()) { return *it; }
        else { return NULL; }
      }
    }
    return NULL;
  }

  const std::string chain_name() const { return chain_name_; }
  void set_chain_name(const std::string& name) { chain_name_ = name; }

  std::vector<Processor*>& processors() {
    return processors_;
  }

 private:
  std::vector<Processor*> processors_;
  std::vector<Chain*> sub_chains_;
  std::map<std::string, Variable> processor_params_;
  std::string chain_name_;
  Renderer* renderer_;
};  // Chain

}  // namespace prediction

#endif  // PREDICTION_SERVER_CHAIN_CHAIN_H_

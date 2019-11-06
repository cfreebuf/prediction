// File   exp_layer.cpp
// Author lidongming
// Date   2018-09-27 01:45:51
// Brief

#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <random>

#include "exp_layer.h"
#include "common/common_value.h"
#include "exp_domain.h"
#include "util/logging.h"

namespace prediction {

int RandomInt(int a, int b) {  
  static std::default_random_engine e { std::random_device{  }() };
  static std::uniform_int_distribution<int> u;  
  return u(e, decltype(u)::param_type(a, b));  
}

ExperimentBase* Layer::GetExpByRandom(const RequestContext& request) {
  ExperimentBase* p_current_exp = nullptr;
  int flow_bucket = -1;

  if (_div_type == DIV_UID) {
    std::string aid = request.aid() + _name + _hash_seed;
    flow_bucket = request.get_aid_hash(aid, _hash_base);
    // LOG(INFO) << "flow bucket:" << flow_bucket << " uid:" << aid;
    // flow_bucket = RandomInt(0, _hash_base);
  } else if (_div_type == DIV_RANDOM) {
    flow_bucket = RandomInt(0, _hash_base);
  } else {
    // LOG_ERROR("unsupported div type:%d", _div_type);
    return nullptr;
  }

  for (auto p_exp : _exp_list) {
    if (p_exp == nullptr || !p_exp->InRange(flow_bucket)) {
      LOG(WARNING) << "not hit exp"
        << " bucket:" << flow_bucket
        << " bucket_flow:" << p_exp->GetBucketFlow()
        << " start:" << p_exp->GetBucketStart()
        << " end:" << p_exp->GetBucketEnd();
      continue;
    } else {
      // LOG(INFO) << "hit exp"
      //   << " bucket:" << flow_bucket
      //   << " bucket_flow:" << p_exp->GetBucketFlow()
      //   << " start:" << p_exp->GetBucketStart()
      //   << " end:" << p_exp->GetBucketEnd();
    }

    bool matched = p_exp->CheckCondition(request);
    if (!matched) {
      LOG(WARNING) << "not match exp"
        << " bucket:" << flow_bucket
        << " bucket_flow:" << p_exp->GetBucketFlow()
        << " start:" << p_exp->GetBucketStart()
        << " end:" << p_exp->GetBucketEnd();
      continue;
    }

    if (p_current_exp == nullptr) {
      p_current_exp = p_exp;
    } else {
      Condition* p_condition = (Condition*)p_exp->GetConditionsPtr();
      Condition* p_cur_condition = (Condition*)p_current_exp->GetConditionsPtr();

      Condition::ConditionStatus status = p_cur_condition->IsSameDimension(*p_condition);
      if (status == Condition::SMALLER) {
        p_current_exp = p_exp;
        // LOG_INFO("exp:%s has a larger priority condition than exp:%s",
        // p_exp->name().c_str(),
          // p_current_exp->name().c_str());
      } else if (status != Condition::LARGER) {
        // LOG_ERROR("expertiment_framework condition of exps must be wrong,%s,%s",
        // p_condition->ToString().c_str(),
          // p_cur_condition->ToString().c_str());
        p_current_exp = nullptr;
        break;
      }
    }
  }
  // if (!p_current_exp) {
  //     // LOG_ERROR("exp is null bucket:%d", flow_bucket);
  // }
  return p_current_exp;
}

ExperimentBase* Layer::GetExpByName(const std::string& exp_id) {
  for (auto p_exp : _exp_list) {
    if (p_exp == nullptr) {
      continue;
    }
    if (p_exp->GetType() == Domain::TYPE_DOMAIN) {
      Domain* p_domain = dynamic_cast<Domain *>(p_exp);
      // std::shared_ptr<Domain> p_domain = std::dynamic_pointer_cast<std::shared_ptr<Domain>>(p_exp);
      // std::shared_ptr<Domain> p_domain = std::dynamic_pointer_cast<Domain>(p_exp);
      if (p_domain->HasExp(exp_id)) {
        return p_exp;
      }
    } else {
      if (p_exp->GetName() == exp_id) {
        return p_exp;
      }
    }
  }
  return nullptr;
}

int Layer::DoExp(const RequestContext& request,
    std::list<Experiment*>& experiments,
    Parameters& parameters) {
  // LOG(INFO) << "Layer::do_exp start:" <<_name;

  ExperimentBase* p_current_exp = nullptr;

  // exp by url
  auto& force_expids = request.GetForceExpIds();
  bool force_exp = force_expids.empty() ? false : true;
  if (force_exp) {
    for (const std::string exp_id: force_expids) { // find first force expid and break
      p_current_exp = GetExpByName(exp_id);
      if (p_current_exp != nullptr) {
        break;
      }
    }
  }
  if (p_current_exp == nullptr) {
    p_current_exp = GetExpByRandom(request);
  }

  if (p_current_exp == nullptr) {
  } else if (p_current_exp->GetType() == ExperimentBase::TYPE_DOMAIN) {
    Domain * p_domain = dynamic_cast<Domain *>(p_current_exp);
    // std::shared_ptr<Domain> p_domain = std::dynamic_pointer_cast<Domain>(p_current_exp);
    return p_domain->DoExp(request, experiments, parameters);
  } else { // experiment
    Experiment * p_exp = dynamic_cast<Experiment *>(p_current_exp);
    // std::shared_ptr<Experiment> p_exp = std::dynamic_pointer_cast<Experiment>(p_current_exp);
    experiments.push_back(p_exp);

    // collect parameters ...
    Parameters& params = p_exp->GetParameters();
    for (auto & kv : params.get_params()) {
      parameters.set_value(kv.first, kv.second);
      parameters.set_source(kv.first, p_exp->GetName());
      // LOG(INFO) << "set exp param key:" << kv.first << " value:" << kv.second;
    }
  }

  return 0;
}

} /* namespace prediction */

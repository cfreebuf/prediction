// CopyRight 2019 360. All rights reserved.
// File   multi_layer_exp_handler.cpp
// Date   2019-10-29 11:23:07
// Brief

#include <vector>
#include "multi_layer_exp_handler.h"
#include "request_context.h"
#include "common/common_value.h"
#include "util/string_utils.h"
#include "util/logging.h"

namespace prediction {

MultiLayerExpHandler::MultiLayerExpHandler() : _init(false) { }
MultiLayerExpHandler::~MultiLayerExpHandler() { }

int MultiLayerExpHandler::Reload(const std::string& path) {
    int ret = _framework.ReloadConfig(path);
    if (ret == 0 && _init == false) {
        _init = true;
    }
    return ret;
}

bool MultiLayerExpHandler::GetInitStatus() { return _init; }

int MultiLayerExpHandler::Init() {
    int ret = _framework.LoadConfig(CONF_FILE_PATH);
    if (ret == 0) { _init = true; }
    return ret;
}

int MultiLayerExpHandler::DoExp(Request& request) {
    if (!_init) { return -1; }

    RequestContextImpl rc(request);

    std::list<Experiment*> experiments;
    Parameters params;
    int ret = _framework.DoExp(rc, experiments, params);
    if (ret != 0) {
        LOG(WARNING) << "do exp error";
        return ret;
    }

    for (auto & kv : params.get_params()) {
        request.Set(kv.first, kv.second);
        // LOG(INFO) << "param key:" << kv.first << " value:" << kv.second;
    }

    std::string exp_ids = "";
    std::string alter_exp_ids = "";
    std::string hit_domain_name = params.get_value("hit_domain_name");
    bool first = true;
    bool alter_first = true;
    for (auto & exp : experiments) {
        if (!first) { exp_ids += ","; }
        exp_ids += exp->GetName();
        first = false;

        if (!exp->GetAlterName().empty()) {
            if (!alter_first) { alter_exp_ids += ","; }
            alter_exp_ids += exp->GetAlterName();
            alter_first = false;
        }
    }
    exp_ids += ",";
    exp_ids += hit_domain_name;
    if (alter_exp_ids != "") {
        exp_ids += ";" + alter_exp_ids;
    }
    request.Set(EXP_KEY, exp_ids);
    request.Set(ALTER_EXP_IDS, alter_exp_ids);
    // LOG(INFO) << "hit exp_ids:" << exp_ids;

    return 0;
}

RequestContextImpl::RequestContextImpl(Request& request) : _request(request) {
    _aid = request.Get("uid");
    std::string aid = request.Get("aid");
    if (!aid.empty()) {
        _aid = aid;
    }

    std::string raw_force_exp_id = request.Get("exp");
    std::vector<std::string> tmp_vec;
    util::StringUtils::Split(raw_force_exp_id, ',', tmp_vec);
    for (std::string & s: tmp_vec) {
      util::StringUtils::Trim(s);
        if (!s.empty()) {
            _force_expids.push_back(s);
        }
    }
}

RequestContextImpl::~RequestContextImpl() { }

std::string RequestContextImpl::Get(const std::string& key,
                                    const std::string& default_value) const {
    return _request.Get(key, default_value);
}

std::string RequestContextImpl::aid() const { return _aid; }

const std::vector<std::string>& RequestContextImpl::GetForceExpIds() const {
    return _force_expids;
}

}

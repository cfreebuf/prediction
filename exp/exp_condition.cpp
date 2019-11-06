// CopyRight 2019 360. All rights reserved.
// File   exp_condition.cpp
// Date   2019-10-28 15:00:37
// Brief

#include "exp_condition.h"
#include <set>
#include "request_context.h"
#include "common/common_define.h"
#include "util/logging.h"

namespace prediction {

std::string add_beg_end_reg_symbol(const std::string &s) {
    return "^(" + s + ")$";
}

Condition::ConditionStatus Condition::IsSameDimension(const Condition& rhs_condition) {
    ConditionStatus ret = CONFLICTION;
    std::set<std::string> tmp_condition;
    int lhs_size = _conditions.size();
    int rhs_size = rhs_condition._conditions.size();
    std::map<std::string, std::string>::const_iterator iter
        = rhs_condition._conditions.begin();
    for (; iter != rhs_condition._conditions.end(); ++iter) {
        if (iter->second == "*") {
            rhs_size -= 1;
            continue;
        }
        tmp_condition.insert(iter->first);
    }
    iter = _conditions.begin();
    for (; iter != _conditions.end(); ++iter) {
        if (iter->second == "*") {
            lhs_size -= 1;
            continue;
        }
        tmp_condition.insert(iter->first);
    }
    int max_size = lhs_size > rhs_size ? lhs_size : rhs_size;
    if (max_size != (int) tmp_condition.size()) {
        ret = CONFLICTION;
    } else {
        if (lhs_size == rhs_size) {
            ret = EQUAL;
            iter = rhs_condition._conditions.begin();
            for (; iter != rhs_condition._conditions.end(); ++iter) {
                auto iter2 = _conditions.find(iter->first);
                if (iter2 != _conditions.end() && iter->second != iter2->second) {
                    ret = SAME_DIMENSION;
                    break;
                }
            }
        } else if (lhs_size > rhs_size) {
            ret = LARGER;
        } else {
            ret = SMALLER;
        }
    }
    return ret;
}

Condition::Condition() { }

Condition::Condition(const std::map<std::string, std::string>& conditions) {
    _conditions = conditions;
    std::map<std::string, std::string>::const_iterator it = conditions.begin();
    for (; it != conditions.end(); ++it) {
       std::shared_ptr<Regex> p_regex = std::make_shared<Regex>(add_beg_end_reg_symbol(it->second));
       _condition_ptrs[it->first] = p_regex;
    }
}

Condition::Condition(const Condition& rhs) {
    _conditions.clear();
    std::map<std::string, std::shared_ptr<Regex>>::iterator it = _condition_ptrs.begin();
    for (; it != _condition_ptrs.end(); ++it) {
        std::shared_ptr<Regex> p_regex = it->second;
        // if (p_regex != NULL) { delete p_regex; }
        p_regex = NULL;
    }
    _condition_ptrs.clear();

    _conditions = rhs._conditions;
    std::map<std::string, std::string>::iterator iter = _conditions.begin();
    for (; iter != _conditions.end(); ++iter) {
        std::shared_ptr<Regex> p_regex = std::make_shared<Regex>(add_beg_end_reg_symbol(iter->second));
        _condition_ptrs[iter->first] = p_regex;
    }
}

Condition::~Condition() {
    _conditions.clear();
    _condition_ptrs.clear();

    // std::map<std::string, std::shared_ptr<Regex>>::iterator it = _condition_ptrs.begin();
    // for (; it != _condition_ptrs.end(); it++) {
    //     std::shared_ptr<Regex> p_regex = it->second;
    //     if (p_regex != NULL) {
    //         delete p_regex;
    //     }
    //     p_regex = NULL;
    // }
}

void Condition::SetCondition(const std::string & key, const std::string & value) {
    _conditions[key] = value;
    std::shared_ptr<Regex> p_regex_old = _condition_ptrs[key];
    // if (p_regex_old != NULL) { delete p_regex_old; }
    p_regex_old = NULL;
    std::shared_ptr<Regex> p_regex = std::make_shared<Regex>(add_beg_end_reg_symbol(value));
    _condition_ptrs[key] = p_regex;
}

void Condition::SetConditions(const std::map<std::string, std::string>& conditions) {
    std::map<std::string, std::string>::const_iterator it = conditions.begin();
    for (; it != conditions.end(); ++it) {
        _conditions[it->first] = it->second;
        std::shared_ptr<Regex> p_regex_old = _condition_ptrs[it->first];
        // if (p_regex_old != NULL) { delete p_regex_old; }
        p_regex_old = NULL;
        std::shared_ptr<Regex> p_regex = std::make_shared<Regex>(add_beg_end_reg_symbol(it->second));
        _condition_ptrs[it->first] = p_regex;
    }
}

const std::string Condition::ToString() const {
    std::string str = "[";
    std::map<std::string, std::string>::const_iterator it = _conditions.begin();
    for (; it != _conditions.end();) {
        str += it->first + ":" + it->second;
        it++;
        if (it != _conditions.end()) {
            str += ",";
        }
    }
    str += "]";
    return str;
}

bool Condition::MatchReq(const RequestContext & req) {
    bool ret = true;
    for (auto& kv : _condition_ptrs) {
        const std::string value = req.Get(kv.first);
        ret = kv.second->Match(value);
        // LOG(INFO) << "rid:" << req.Get("rid") << " ret:" << ret
        //           << " key:" << kv.first
        //           << " pattern:" << kv.second->GetPattern()
        //           << " value:" << value;
        // FIXME(lidongming): check req 返回为空时，是否会匹配不成功
        if (!ret) { break; }
    }
    return ret;
}

std::string Condition::GetCondition(const std::string& key) {
    std::string ret = "";
    auto it = _conditions.find(key);
    if (it != _conditions.end()) { return it->second; }
    return ret;
}

std::shared_ptr<Regex> Condition::GetConditionRegex(const std::string& key) {
    std::shared_ptr<Regex> ret = NULL;
    auto it = _condition_ptrs.find(key);
    if (it != _condition_ptrs.end()) { ret = it->second; }
    return ret;
}

}  // namespace prediction

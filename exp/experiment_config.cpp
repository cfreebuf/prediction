// File   experiment_config.cpp
// Author lidongming
// Date   2018-09-26 23:52:15
// Brief

#include <libconfig.h++>
#include "experiment_config.h"
#include <utility>
#include "config_load_helper.h"
#include "util/logging.h"

namespace prediction {

using namespace libconfig;

ExperimentConfig::ExperimentConfig() : _root_hash_base(0), _ref_count(0) {
    _config_load_helper = std::make_shared<ConfigLoadHelper>();
}

ExperimentConfig::~ExperimentConfig() { } 

/**
 * Load config from files. The directory structure should be:
 *
 * <exp_path>
 *  |- root.entry   // the main entry. the top domains are defined in this file.
 *  |               // for each domain, some layers are defined. and the layer
 *  |               // meta is defined in layer file (see below)
 *  |- xxx.layer    // layer definition, in which layer meta and experiments
 *  |               // are defined
 *
 * The config loading process consists of two phases:
 *
 *  phase 1: config files loaded and converted to XXConfigMeta. In this phase,
 *           config file syntax is checked
 *
 *  phase 2: ConfigMeta converted into Config objects. In this phase, experiment
 *           semantic logic is validated
 *
 */
int ExperimentConfig::LoadConfig(const std::string& config_path) {
    int ret = 0;

    _config_load_helper->_config_path = config_path;
    //load default params
    ret = _config_load_helper->LoadDefaultParams(_default_params);
    if (ret != LOAD_SUCCESS) {
        LOG(WARNING) << "load default params error";
        return ret ;
    }

    RootEntryConfigMeta root_meta;
    ret = _config_load_helper->LoadRootEntry(root_meta);
    if (ret != 0) {
        LOG(WARNING) << "load root entry error";
        return ret;
    }

    _config_path = config_path;

    ret = BuildConfig(root_meta);
    if (ret != 0) {
        LOG(WARNING) << "build config error";
        return ret;
    }

    return 0;
}

int ExperimentConfig::BuildExperiment(const ExpConfigMeta& exp_meta,
                                      Experiment& experiment,
                                      const std::string& layer_name,
                                      int bucket_start) {
    experiment.SetName(exp_meta.name);
    experiment.SetAlterName(exp_meta.alter_name);
    experiment.SetBucketStart(bucket_start);
    experiment.SetBucketEnd(bucket_start + exp_meta.bucket_flow - 1);
    experiment.SetBucketFlow(exp_meta.bucket_flow);
    bucket_start += exp_meta.bucket_flow;

    // experiment.set_single_tail_number(exp_meta.single_tail_number);
    // experiment.set_mul_tail_number(exp_meta.mul_tail_number);

    const std::map<std::string, std::string>& conditions = exp_meta.get_conditions();
    experiment.SetConditions(conditions);

    for (auto& kv : exp_meta.params) {
        experiment.GetParameters().set_value(kv.first, kv.second);
    }

    return 0;
}

int ExperimentConfig::ConditionChecker(std::vector<Condition>& condition_vec,
                                         std::vector<int>& condition_range,
                                         const LayerConfigMeta& layer_meta,
                                         const ExpConfigMetaBasePtr& exp_meta,
                                         int& bucket_start) {
    int cur_range = exp_meta->bucket_flow;
    Condition cur_condition(exp_meta->get_conditions());

    bool find_same_conditon = false;
    for (int i = 0; i < (int)condition_vec.size(); i++) {
        Condition & i_condition = condition_vec[i];
        Condition::ConditionStatus status = cur_condition.IsSameDimension(i_condition);
        if (status == Condition::EQUAL) {
            find_same_conditon = true;
            bucket_start = condition_range[i];
            condition_range[i] = condition_range[i] + cur_range;
            if (condition_range[i] > layer_meta.get_hash_base()) {
                return -1;
            }
        } else if (status == Condition::CONFLICTION) {
            // LOG_ERROR("condition:%s conflicts with condition:%s",
                       // cur_condition.ToString().c_str(),
                       // i_condition.ToString().c_str());
            return -1;
        }
    }
    if (find_same_conditon == false) {
        condition_vec.push_back(cur_condition);
        condition_range.push_back(cur_range);
        bucket_start = 0;
    }
    return 0;
}

int ExperimentConfig::CheckLayer(const Layer& layer, Domain& domain) {
    std::set<std::string>& domain_exps = domain.GetDomainExps();
    std::set<std::string>& params = domain.GetDomainParams();
    std::set<std::string> layer_params;
    const std::vector<ExperimentBase*> exps = layer.get_experiments();
    
    for(const auto& exp:exps) {
        if (exp->GetType() == ExperimentBase::TYPE_DOMAIN) {
            Domain* exp_ptr = dynamic_cast<Domain *> (exp);
            // std::shared_ptr<Domain> exp_ptr = std::dynamic_pointer_cast<Domain> (exp);
            auto& domain_params = exp_ptr->GetDomainParams();
            layer_params.insert(domain_params.begin(), domain_params.end());
            exp_ptr->ResetDomainParams();//domain_params clear

            //set domain exps for check force expid
            auto& sub_domain_exps = exp_ptr->GetDomainExps();
            domain_exps.insert(sub_domain_exps.begin(),sub_domain_exps.end());
        } else {
            Experiment* exp_ptr = dynamic_cast<Experiment *> (exp);
            // std::shared_ptr<Experiment> exp_ptr = std::dynamic_pointer_cast<Experiment>(exp);
            const auto& exp_params = exp_ptr->GetParameters().get_params();
            // for (const auto& p : exp_params) {
            for (const auto p : exp_params) {
                layer_params.insert(p.first);
            }
            //set domain exps
            domain_exps.insert(exp_ptr->GetName());
        }
    }
    
    std::string log;
    for (const auto& layer_param:layer_params) {
        if (params.find(layer_param) == params.end()) {
            params.insert(layer_param);
            log += (layer_param + ",");
        } else {
            // LOG_ERROR("expertiment_framework param conflicts [%s]",layer_param.c_str());
            return -1;
        }
    }
    // // LOG_DEBUG("layer = %s, layer_param = %s",layer.GetName().c_str(),log.c_str());

    return 0;

}

int ExperimentConfig::BuildLayer(const LayerConfigMeta& layer_meta, Layer& layer) {
    int ret = 0;

    layer.SetName(layer_meta.name);

    int hash_base = layer_meta.hash_base;
    layer.set_hash_base(hash_base);
    layer.set_hash_seed(layer_meta.hash_seed);

    DiversionType div_type = DiversionTypeUtils::from_string(layer_meta.diversion_type);
    if (div_type == DIV_UNKNOWN) {
        // LOG_ERROR("unknown diversion type:%s in layer:%s",
                   // layer_meta.diversion_type.c_str(), layer_meta.name.c_str());
        return -1;
    }
    layer.set_div_type(div_type);

    int bucket_start = 0;

    std::vector<Condition> condition_vec;
    std::vector<int> condition_range;
    std::set<std::string> layer_name_set; // for duplicated experiment
    bool layername_hit_in_domain = false;

    for (auto& exp_meta : layer_meta.experiments) {
        ret = ConditionChecker(condition_vec, condition_range, layer_meta,
                               exp_meta, bucket_start);
        if (ret < 0) { return -1; }

        if (exp_meta->get_type() == ExperimentBase::TYPE_DOMAIN) {
            // Domain
            DomainConfigMetaPtr p_domain_meta = std::static_pointer_cast<DomainConfigMeta>(exp_meta);
            Domain domain;

            ret = BuildDomain(*p_domain_meta, domain, bucket_start);
            if (ret != 0) {
                // LOG_ERROR("build domain [%s] failed", p_domain_meta->name.c_str());
                return ret;
            }
            // bucket_start = domain.get_bucket_end() + 1;
            if (bucket_start > hash_base) {
                // LOG_ERROR("sum of bucket_flow exceeds the hash base");
                return -1;
            }
            ret = _domain_registry.add(domain);
            if (ret != 0) {
                // LOG_ERROR("register domain [%s] failed", domain.GetName().c_str());
                return ret;
            } else {
               // // LOG_INFO("register domain [%s] success", domain.GetName().c_str());
            }

            // std::shared_ptr<Domain> p_domain = _domain_registry.get_element(p_domain_meta->name);
            Domain * p_domain = _domain_registry.get_element(p_domain_meta->name);
            if (p_domain != nullptr) {
                layer.add_experiment(p_domain);
            }
        } else {
            ExpConfigMetaPtr p_exp_meta = std::static_pointer_cast<ExpConfigMeta>(exp_meta);
            Experiment * p_exp = _experiment_registry.get_element(p_exp_meta->name);
            // std::shared_ptr<Experiment> p_exp = _experiment_registry.get_element(p_exp_meta->name);
            layername_hit_in_domain = false;
            if (layer_name_set.count(p_exp_meta->name) > 0) {
                layername_hit_in_domain = true;
            }
            if (p_exp != nullptr && layername_hit_in_domain) { // already exist
                // LOG_ERROR("duplicated experiment: %s", p_exp_meta->name.c_str());
                return -1;
            }
            
            layer_name_set.insert(p_exp_meta->name);
            Experiment experiment;
            ret = BuildExperiment(*p_exp_meta, experiment, p_exp_meta->name, bucket_start);
            if (ret != 0) {
                // LOG_ERROR("build experiment [%s] failed", p_exp_meta->name.c_str());
                return ret;
            }
            // bucket_start = experiment.get_bucket_end() + 1;
            if (bucket_start > hash_base) {
                // LOG_ERROR("sum of bucket_flow exceeds the hash base");
                return -1;
            }
            // until here, an experiment object is built
            ret = _experiment_registry.add(experiment);
            if (ret != 0 && layername_hit_in_domain) { // already exist
                // LOG_ERROR("register experiment [%s] failed", experiment.GetName().c_str());
                return ret;
            } else {
                ret = 0;
               // // LOG_INFO("register experiment [%s] success", experiment.GetName().c_str());
            }
            p_exp = _experiment_registry.get_element(p_exp_meta->name);

            if (p_exp != nullptr) {
                layer.add_experiment(p_exp);
            }
        }
    } // end of for(experiments)

    return ret;
}

int ExperimentConfig::BuildDomain(DomainConfigMeta& domain_meta,
                                    Domain& domain, int bucket_start) {
    int ret = 0;

    domain.SetName(domain_meta.name);
    domain.SetBucketStart(bucket_start);
    domain.SetBucketEnd(bucket_start + domain_meta.bucket_flow - 1);
    domain.SetBucketFlow(domain_meta.bucket_flow);
    std::set<std::string> domain_name_set;

    for (LayerConfigMeta& layer_meta : domain_meta.layers) {
        Layer layer;
        ret = BuildLayer(layer_meta, layer);
        if (ret != 0) {
            // LOG_ERROR("build layer [%s] failed", layer_meta.name.c_str());
            return ret;
        }

        ret = CheckLayer(layer, domain);
        if (ret != 0) {
            // LOG_ERROR("check layer [%s] failed", layer_meta.name.c_str());
            return ret;
        }
        ret = _layer_registry.add(layer);

        if (ret != 0 && domain_name_set.count(layer.GetName()) > 0) {
            // LOG_ERROR("register layer [%s] failed", layer_meta.name.c_str());
            return ret;
        }
        domain_name_set.insert(layer.GetName());
        ret = 0;

        Layer * p_layer = _layer_registry.get_element(layer_meta.name);
        // std::shared_ptr<Layer> p_layer = _layer_registry.get_element(layer_meta.name);
        if (p_layer != nullptr) {
            domain.AddLayer(p_layer);
        }
    }

    const std::map<std::string, std::string>& conditions = domain_meta.get_conditions();
    domain.SetConditions(conditions);

    return ret;
}

int ExperimentConfig::BuildConfig(RootEntryConfigMeta& root_entry) {
    int ret = 0;

    _root_hash_base = root_entry.hash_base;
    _root_hash_seed = root_entry.hash_seed;

    // launch layers
    for (LayerConfigMeta& layer_meta : root_entry.launcher_layers) {
        Layer layer;
        ret = BuildLayer(layer_meta, layer);
        if (ret != 0) {
            LOG(WARNING) << "build layer " << layer_meta.name << " failed";
            return ret;
        }
        ret = _layer_registry.add(layer);
        if (ret != 0) {
            LOG(WARNING) << "register layer " << layer_meta.name << " failed";
            return ret;
        }

        // std::shared_ptr<Layer> p_layer = _layer_registry.get_element(layer_meta.name);
        Layer * p_layer = _layer_registry.get_element(layer_meta.name);
        if (p_layer != nullptr) {
            _launch_layers.push_back(p_layer);
        }
    }

    // domains
    int domain_bucket_start = 0;
    for (DomainConfigMeta & domain_meta : root_entry.domains) {
        Domain domain;
        ret = BuildDomain(domain_meta, domain, domain_bucket_start);
        if (ret != 0) {
            LOG(WARNING) << "build domain " << domain_meta.name << " failed";
            return ret;
        }

        //remove domain params for check
        domain.ResetDomainParams();

        if (domain.GetBucketFlow() != _root_hash_base) {
            LOG(WARNING) << "each entry domain bucket flow must be equal to the hash base " << domain_meta.name;
            return -1;
        }

        ret = _domain_registry.add(domain);
        if (ret != 0) {
            LOG(WARNING) << "register domain " << domain_meta.name << " failed";
            return ret;
        }

        // std::shared_ptr<Domain> d = _domain_registry.get_element(domain_meta.name);
        Domain* d = _domain_registry.get_element(domain_meta.name);
        if (d != nullptr) {
            _domains.push_back(d);
        }
    }

    return ret;
}

int ExperimentConfig::DoExp(const RequestContext& request,
                            std::list<Experiment*>& experiments,
                            Parameters& parameters) {
    // 1.load default params
    for (const auto & kv : _default_params.get_params()) {
        parameters.set_value(kv.first, kv.second);
        parameters.set_source(kv.first,"default.params");
    }
    // 2.load launch layer param
    for (auto layer : _launch_layers) {
        if (layer == nullptr) { continue; }
        layer->DoExp(request, experiments, parameters);
    }
    // exp from url
    auto& force_expids = request.GetForceExpIds();
    bool force_exp = force_expids.empty() ? false : true;
    
    // domains
    std::string aid_value(request.aid());
    aid_value.append(_root_hash_seed);
    int bucket = request.get_aid_hash(aid_value, _root_hash_base);

    // std::shared_ptr<Domain> domain = nullptr;
    Domain* domain = nullptr;
    // get domain for force exp
    if (force_exp) {
        std::string domain_name = force_expids.back();
        for (auto d : _domains) {
            if (d == nullptr) { continue; }
            if (domain_name == d->GetName()) {
                domain = d;
                break;
            }
        }
    }
    // get domain by random
    if (domain == nullptr) {
        for (auto d : _domains) {
            if (d == nullptr) { continue; }
            if (d->InRange(bucket) && d->CheckCondition(request)) {
                domain = d;
                break;
            } 
        }
    }

    if (domain == nullptr) {
        LOG(WARNING) << "no domain could be triggered";
    } else {
        parameters.set_value("hit_domain_name", domain->GetName());
        for (auto layer : domain->get_layers()) {
            if (layer == nullptr) { continue; }
            std::string old_exp_layers = parameters.get_value("exp_layers");
            if (old_exp_layers.empty()) {
                parameters.set_value("exp_layers", layer->GetName());
            } else {
                parameters.set_value("exp_layers", old_exp_layers + "," + layer->GetName());
            }
            layer->DoExp(request, experiments, parameters);
        }
    }
    return 0;
}

} /* namespace prediction */

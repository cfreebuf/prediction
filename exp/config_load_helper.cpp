// File   config_load_helper.cpp
// Author lidongming
// Date   2018-09-26 23:45:04
// Brief

#include "config_load_helper.h"
// #include <string>
#include "experiment_elements.h"
#include "util/logging.h"

namespace prediction {

using namespace libconfig;

#define CHECK_REQUIRED_FIELD(element, field) \
    do { \
        if (!element.exists(field)) { \
            LOG(WARNING) << "no " << field << " defined"; \
            return -1; \
        } \
    } while (0)

ConfigLoadHelper::ConfigLoadHelper() { }
ConfigLoadHelper::~ConfigLoadHelper() { }

int ConfigLoadHelper::LoadDefaultParams(Parameters& default_params) {
    std::string default_param_file = _config_path + "/" + DEFAULT_PARAMS;
    libconfig::Config config;
    try {
        config.readFile(default_param_file.c_str());
        default_params.reset();
        Setting& root = config.getRoot();

        CHECK_REQUIRED_FIELD(root, "default");

        Setting& params_element = root["default"];
        if (params_element.getType() != libconfig::Setting::TypeGroup || 
                params_element.getLength() < 1) {
            LOG(WARNING) << "expertiment_framework default.params type error";
            return LOAD_INVALID_FORMAT;
        }

        int len = params_element.getLength();
        for(int i=0 ; i < len ; ++i) {
            Setting & param_pair = params_element[i];
            const char * key = param_pair.getName();
            std::string value;
            Setting2Str(param_pair, value);  
            default_params.set_value(key,value);
        }
        if (!default_params.has_key("chain")
                || !default_params.has_key("renderer")) {
            LOG(FATAL) << "expertiment_framework default.params has no chain or renderer";
            return LOAD_INVALID_FORMAT;
        }
        // LOG(INFO) << "default chain:" << default_params.get_value("chain");

    } catch (...) {
        LOG(FATAL) << "expertiment_framework read config file error:"
                   << default_param_file;
        return LOAD_INVALID_FORMAT;
    }
    return LOAD_SUCCESS;
}

int ConfigLoadHelper::LoadRootEntry(RootEntryConfigMeta& root_meta) {
    std::string root_entry_file = _config_path + "/" + ROOT_ENTRY;

    libconfig::Config config;
    int ret = LOAD_SUCCESS;

    try {
        config.readFile(root_entry_file.c_str());
    }
    catch (std::exception& ex) {
      LOG(FATAL) << "expertiment_framework read config file:" << root_entry_file
                 << " error:" << ex.what();
        return LOAD_INVALID_FORMAT;
    }

    root_meta.reset();

    Setting& root = config.getRoot();

    CHECK_REQUIRED_FIELD(root, "domains");

    if (root.exists("launch_layers")) {
        Setting& launcher_element = root["launch_layers"];
        if (launcher_element.getType() != Setting::TypeArray) {
            // LOG_WARNING("expertiment_framework wrong type of launch_layers");
            return -1;
        }

        int len = launcher_element.getLength();
        LayerConfigMeta layer_meta;

        for (int i = 0; i < len; i++) {
            Setting& layer_element = launcher_element[i];
            if (layer_element.getType() != Setting::TypeString) {
                return -1;
            } else {
                std::string layer_name = (const char *) layer_element;
                root_meta.launcher_layer_names.push_back(layer_name);
                layer_meta.reset();
                ret = LoadLayerFromFile(layer_name, layer_meta);
                if (ret != 0) {
                  LOG(FATAL) << "expertiment_framework load layer error:"
                             << layer_name;
                    return -1;
                }
                root_meta.launcher_layers.push_back(layer_meta);
            }
        }
    }

    CHECK_REQUIRED_FIELD(root, "hash_base");

    root_meta.hash_base = (int) root["hash_base"];
    if (root.exists("hash_seed")) {
        Setting2Str(root["hash_seed"], root_meta.hash_seed);
    }

    Setting& domain_elements = root["domains"];
    if (domain_elements.getType() != Setting::TypeList) {
        LOG(FATAL) << "expertiment_framework wrong type of domains";
        return -1;
    }
    int domain_len = domain_elements.getLength();
    DomainConfigMeta domain_meta;
    for (int i = 0; i < domain_len; i++) {
        Setting& domain_element = domain_elements[i];
        domain_meta.reset();
        ret = LoadDomainFromElement(domain_element, domain_meta);
        if (ret != 0) {
            LOG(FATAL) << "expertiment_framework load domain error:"
                       << root_entry_file;
            return ret;
        }
        root_meta.domains.push_back(domain_meta);
    }

    return 0;
}

int ConfigLoadHelper::LoadLayerFromFile(const std::string & layer_name,
                                        LayerConfigMeta & layer_meta) {
    std::string layer_config = _config_path + "/" + layer_name + LAYER_SUFFIX;
    // LOG_DEBUG("loading layer: %s", layer_config.c_str());

    int ret = 0;

    libconfig::Config config;
    try {
        config.readFile(layer_config.c_str());
    } catch (...) {
        // LOG_ERROR("expertiment_framework read config file [%s] error",
                   // layer_config.c_str());
        return -1;
    }

    layer_meta.name = layer_name;
    Setting& root = config.getRoot();
    if (!root.exists("diversion_type")) {
        // LOG_WARNING("expertiment_framework no diversion type defined");
        return -1;
    }
    layer_meta.diversion_type = (const char *) root["diversion_type"];

    if (!root.exists("hash_base")) {
        // LOG_WARNING("expertiment_framework no hash_base defined");
        return -1;
    }
    layer_meta.hash_base = (int) root["hash_base"];
    int total_base = (int) root["hash_base"];

    if (root.exists("hash_seed")) {
        Setting2Str(root["hash_seed"], layer_meta.hash_seed);
    }

    if (!root.exists("exp_list")) {
        // LOG_WARNING("expertiment_framework no exp_list defined");
        return -1;
    }

    Setting& exp_list_elements = root["exp_list"];
    int len = exp_list_elements.getLength();

    for (int i = 0; i < len; i++) {
        // LOG_DEBUG("loading exp:%d in layer:%s", i, layer_name.c_str());
        Setting& exp_element = exp_list_elements[i];
        ret = AddExpElement(exp_element, layer_meta);
        if (ret != 0) {
            // LOG_WARNING("expertiment_framework load experiment [%d] "
                        // "in layer [%s] failed", i + 1, layer_config.c_str());
            return ret;
        }
        total_base -= (int) exp_element["bucket_flow"];
    }

    if (total_base != 0){
        // LOG_WARNING("hash_bash is not equal the sum of bucket_flow");
        return -1;
    }

    return 0;
}

int ConfigLoadHelper::LoadDomainFromElement(Setting& domain_element,
                                            DomainConfigMeta & domain_meta) {
    CHECK_REQUIRED_FIELD(domain_element, "bucket_flow");
    CHECK_REQUIRED_FIELD(domain_element, "name");
    CHECK_REQUIRED_FIELD(domain_element, "layers");

    domain_meta.bucket_flow = domain_element["bucket_flow"];
    domain_meta.name = (const char *) domain_element["name"];

    Setting& layer_elements = domain_element["layers"];

    int len = layer_elements.getLength();
    if (layer_elements.getType() != Setting::TypeArray || len <= 0
        || layer_elements[0].getType() != Setting::TypeString) {
        // LOG_WARNING("expertiment_framework layers type error");
        return -1;
    }

    if (domain_element.exists("conditions")) {
        // LOG_DEBUG("loading conditions in domain...");
        Setting& condition_element = domain_element["conditions"];
        if (condition_element.getType() != Setting::TypeGroup) {
            // LOG_WARNING("expertiment_framework condition type in domain error");
            return -1;
        }
        int condition_len = condition_element.getLength();
        for ( int i = 0; i < condition_len; i++) {
            Setting & cond_pair = condition_element[i];
            const char * key = cond_pair.getName();
            const char * value = cond_pair;
            domain_meta.conditions.insert(std::make_pair<std::string, std::string>(key, value));
        }
    }

    int ret = 0;
    const char * layer_name = nullptr;
    LayerConfigMeta layer_meta;
    for (int i = 0; i < len; i++) {
        layer_name = layer_elements[i];

        domain_meta.layer_names.push_back(layer_name);
        layer_meta.reset();
        ret = LoadLayerFromFile(layer_name, layer_meta);
        if (ret != 0) {
            // LOG_WARNING("NEW_EXPERTIMENT_FRAMEWORK load layer [%s] failed", layer_name);
            return ret;
        }
        domain_meta.layers.push_back(layer_meta);
    }

    return 0;
}

int ConfigLoadHelper::AddExpElement(Setting& exp_element,
                                    LayerConfigMeta& layer_meta) {
    int ret = 0;

    CHECK_REQUIRED_FIELD(exp_element, "name");
    CHECK_REQUIRED_FIELD(exp_element, "bucket_flow");

    bool is_domain = false;

    if (exp_element.exists("type")) {
        Setting& type_element = exp_element["type"];
        if (type_element.getType() == Setting::TypeString) {
            std::string type = (const char*) type_element;
            if (type == "domain") {
                is_domain = true;
            }
        }
    }

    std::string name = (const char*) exp_element["name"];

    if (is_domain) {
        // LOG_DEBUG("loading domain [%s] in layer", name.c_str());
        DomainConfigMetaPtr p_domain = layer_meta.add_domain_config();
        ret = LoadDomainFromElement(exp_element, *p_domain);
        if (ret != 0) {
            // LOG_WARNING("expertiment_framework load domain failed");
            return ret;
        }
    } else {
        // LOG_DEBUG("loading experiment [%s] in layer", name.c_str());
        CHECK_REQUIRED_FIELD(exp_element, "owner");
        CHECK_REQUIRED_FIELD(exp_element, "description");

        ExpConfigMetaPtr p_exp = layer_meta.add_exp_config();

        p_exp->name = (const char *) exp_element["name"];
        p_exp->bucket_flow = (int) exp_element["bucket_flow"];
        p_exp->owner = (const char *) exp_element["owner"];
        p_exp->desc = (const char *) exp_element["description"];
        
        if (exp_element.exists("alter_name")) {
            Setting& alter_element = exp_element["alter_name"];
            if (alter_element.getType() == Setting::TypeString) {
                p_exp->alter_name = (const char*) alter_element;
            }
        }

        if (exp_element.exists("conditions")) {
            Setting& condition_element = exp_element["conditions"];
            if (condition_element.getType() != Setting::TypeGroup) {
                // LOG_WARNING("expertiment_framework condition type error");
                return -1;
            }

            int condition_len = condition_element.getLength();
            for (int i = 0; i < condition_len; i++) {
                Setting& cond_pair = condition_element[i];
                const char* key = cond_pair.getName();
                const char* value = cond_pair;
                p_exp->conditions.insert(std::make_pair<std::string, std::string>(key, value));
            }
        }

        if (exp_element.exists("params")) {
            Setting& params_element = exp_element["params"];
            if (params_element.getType() != Setting::TypeGroup) {
                // LOG_WARNING("expertiment_framework params type error");
                return -1;
            }
            int params_len = params_element.getLength();
            for (int i = 0; i < params_len; i++) {
                Setting& param_pair = params_element[i];
                const char* key = param_pair.getName();
                std::string value;
                Setting2Str(param_pair, value); 
                p_exp->params.insert(std::make_pair<std::string, std::string>(key, std::move(value)));
            }
        }
    }

    return ret;
}

void ConfigLoadHelper::Setting2Str(libconfig::Setting & param,std::string& val) {
    libconfig::Setting::Type param_type = param.getType();
    switch (param_type) {
        case libconfig::Setting::TypeBoolean:
            val = std::to_string((bool) param);
            break;
        case libconfig::Setting::TypeInt64:
        case libconfig::Setting::TypeFloat:
            val = std::to_string((double) param);
            break;
        case libconfig::Setting::TypeInt:
            val = std::to_string((int) param);
            break;
        case libconfig::Setting::TypeString:
            val = std::string((const char *) param);
            break;
        case libconfig::Setting::TypeArray:
        case libconfig::Setting::TypeGroup:
        case libconfig::Setting::TypeList:
        case libconfig::Setting::TypeNone:
        default:
            break;
    }
}

} /* namespace prediction */

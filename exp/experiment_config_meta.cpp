#include "experiment_config_meta.h"

namespace prediction {

void ExpConfigMeta::dump()
{

    // LOG_INFO("****** experiment ********");
    // LOG_INFO("name  = %s", name.c_str());
    // LOG_INFO("owner = %s", owner.c_str());
    // LOG_INFO("desc  = %s", desc.c_str());
    // LOG_INFO("bucket_flow = %d", bucket_flow);
    // LOG_INFO("conditions: {");
    for (auto & kv : conditions) {
        // LOG_INFO("    %s = %s", kv.first.c_str(), kv.second.c_str());
    }
    // LOG_INFO("}");

    // LOG_INFO("params: {");
    for (auto & kv : params) {
        // LOG_INFO("    %s = %s", kv.first.c_str(), kv.second.c_str());
    }
    // LOG_INFO("}");

}

void ExpConfigMeta::reset()
{

    name.clear();
    owner.clear();
    desc.clear();
    bucket_flow = 0;
    conditions.clear();
    params.clear();

}

///////////////////

ExpConfigMetaPtr LayerConfigMeta::add_exp_config()
{
    ExpConfigMetaPtr p(new ExpConfigMeta());
    experiments.push_back(p);
    return p;
}

DomainConfigMetaPtr LayerConfigMeta::add_domain_config()
{
    DomainConfigMetaPtr p(new DomainConfigMeta());
    experiments.push_back(p);
    return p;
}

LayerConfigMeta::~LayerConfigMeta()
{
    experiments.clear();
}

void LayerConfigMeta::dump()
{
    // LOG_INFO("**** layer [%s] ******", name.c_str());
    // LOG_INFO("name  = %s", name.c_str());
    // LOG_INFO("diversion_type  = %s", diversion_type.c_str());
    // LOG_INFO("hash_base  = %d", hash_base);
    for (auto & exp : experiments) {
        exp->dump();
    }
    // LOG_INFO("***** end of layer [%s] *****", name.c_str());
}

/////////////
void DomainConfigMeta::dump()
{
    // LOG_INFO("******* domain [%s]*********", name.c_str());
    // LOG_INFO("name  = %s", name.c_str());
    // LOG_INFO("layers: [");
    for (auto & layer : layer_names) {
        // LOG_INFO("    %s", layer.c_str());
    }
    // LOG_INFO("]// end of layer meta");
    if (!layers.empty()) {
        // LOG_INFO("layers meta:[");
        for (auto & layer : layers) {
            layer.dump();
        }
        // LOG_INFO("]// end of layers meta");
    }
    // LOG_INFO("** end of domain [%s]*******", name.c_str());
}

//////////////////
void RootEntryConfigMeta::dump()
{
    // LOG_INFO("******* root.entry *********");
    // LOG_INFO("hash_base  = %d", hash_base);
    // LOG_INFO("launcher layers: [");
    for (auto & layer : launcher_layers) {
        layer.dump();
    }
    // LOG_INFO("]// end of launcher layer");
    // LOG_INFO("domains: [");
    for (auto & domain : domains) {
        domain.dump();
    }
    // LOG_INFO("]//end of domains");
    // LOG_INFO("**** end of root.entry ****");
}
}


// File   renderer_factory.cpp
// Author lidongming
// Date   2018-09-26 23:13:18
// Brief

#include "renderer/renderer_factory.h"
#include "renderer/renderers.h"

namespace prediction {

RendererFactory::RendererFactory(void* context) : ComponentFactory(context) {
    Init();
}

RendererFactory::~RendererFactory() {
}

#define REGISTER_RENDERER(_renderer_name_, _renderer_class_) \
    RegisterRenderer(#_renderer_name_, new _renderer_class_())

void RendererFactory::Init() {
    REGISTER_RENDERER(default_renderer, DefaultRenderer);
}

Renderer* RendererFactory::GetRenderer(const std::string& renderer_name) {
    auto it = _renderer_mapping.find(renderer_name);
    if (it != _renderer_mapping.end()) {
        return it->second;
    } else {
        return NULL;
    }
}

void RendererFactory::RegisterRenderer(const std::string renderer_name,
                                       Renderer* renderer) {
    _renderer_mapping[renderer_name] = renderer;
}

}  // namespace prediction

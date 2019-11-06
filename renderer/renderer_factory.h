// File   renderer_factory.h
// Author lidongming
// Date   2017-10-23 00:28:53
// Brief

#ifndef PREDICTION_SERVER_RENDERER__RENDERER_FACTORY_H_
#define PREDICTION_SERVER_RENDERER__RENDERER_FACTORY_H_

// #include "framework/renderer.h"
#include <string>
#include <map>
#include "processor/component_factory.h"

namespace prediction {

#if 0
class RendererFactory {
private:
    std::map<std::string, Renderer*> _renderer_mapping;
    RendererFactory();

public:
    static Renderer* GetRenderer(const std::string& renderer_name);
    static void RegisterRenderer(const std::string renderer_name, Renderer* renderer);
    static void Dump();
    static RendererFactory& GetInstance();
    // RendererFactory& GetInstance() {
    //     static RendererFactory _instance;
    //     return _instance;
    // }

    virtual ~RendererFactory();
}; // RendererFactory
#endif  // end #if 0

class Renderer;

class RendererFactory : public ComponentFactory {
public:
    RendererFactory(void* context);
    ~RendererFactory();

    void Init();
    Renderer* GetRenderer(const std::string& renderer_name);
    void RegisterRenderer(const std::string renderer_name, Renderer* renderer);
    // static RendererFactory& GetInstance();

private:
    // DISALLOW_COPY_AND_ASSIGN(RendererFactory);

    std::map<std::string, Renderer*> _renderer_mapping;

}; // RendererFactory

}  // namespace prediction

#endif  // PREDICTION_SERVER_RENDERER__RENDERER_FACTORY_H_

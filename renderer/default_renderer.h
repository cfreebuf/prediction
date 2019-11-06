// File   json_renderer.h
// Author lidongming
// Date   2017-10-23 02:03:17
// Brief

#ifndef PREDICTION_SERVER_RENDERERS_DEFAULT_RENDERER_H_
#define PREDICTION_SERVER_RENDERERS_DEFAULT_RENDERER_H_

#include "renderer/renderer.h"
#include "chain/context.h"

namespace prediction {

class DefaultRenderer : public Renderer {
public:
    DefaultRenderer() {  }
    virtual ~DefaultRenderer() {  }
    int Render(Context& context);

private:

};  // DefaultRenderer

}  // namespace prediction

#endif  // PREDICTION_SERVER_RENDERERS_DEFAULT_RENDERER_H_

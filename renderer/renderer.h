// CopyRight 2019 360. All rights reserved.
// File   renderer.h
// Date   2019-11-01 19:37:16
// Brief

#ifndef PREDICTION_SERVER_RENDERER_RENDERER_H_
#define PREDICTION_SERVER_RENDERER_RENDERER_H_

#include "chain/context.h"

namespace prediction {

class Renderer {
public:
    virtual int Render(Context& context) = 0;
    virtual ~Renderer() {
    }

    virtual std::string GetName() const { return "<renderer>"; }
};  // Renderer

}  // namespace prediction

#endif  // PREDICTION_SERVER_RENDERER_RENDERER_H_

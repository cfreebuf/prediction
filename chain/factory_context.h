// File   factory_context.h
// Author lidongming
// Date   2018-09-25 23:03:02
// Brief

#ifndef PREDICTION_SERVER_CHAIN_FACTORY_CONTEXT_H_
#define PREDICTION_SERVER_CHAIN_FACTORY_CONTEXT_H_

// Factory
#include "processor/processor_factory.h"
#include "renderer/renderer_factory.h"

namespace prediction {

struct FactoryContext {
 ProcessorFactory* processor_factory;
 RendererFactory* renderer_factory;

 FactoryContext() {
   processor_factory = new ProcessorFactory(this);
   renderer_factory  = new RendererFactory(this);
 }

 ~FactoryContext() {
   if (processor_factory) { delete processor_factory; }
   if (renderer_factory) { delete renderer_factory; }
 }
};

}  // namespace prediction

#endif  // PREDICTION_SERVER_CHAIN_FACTORY_CONTEXT_H_

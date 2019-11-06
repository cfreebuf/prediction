#include "exp_domain.h"
#include "exp_layer.h"

namespace prediction {

int Domain::DoExp(const RequestContext& request,
                  std::list<Experiment*>& experiments,
                  Parameters& parameters) {

    for (auto layer : _layers) {
        if (layer == nullptr) {
            continue;
        }
        layer->DoExp(request, experiments, parameters);
    }
    return 0;
}

}

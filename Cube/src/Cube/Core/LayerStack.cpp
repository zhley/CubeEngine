#include "LayerStack.h"

namespace Cube {

    void LayerStack::pushLayer(const std::shared_ptr<Layer>& layer) {
        layers.push_front(layer);
        layer->onAttach();
    }

    void LayerStack::pushOverLayer(const std::shared_ptr<Layer>& layer) {
        layers.push_back(layer);
        layer->onAttach();
    }

    std::shared_ptr<Layer> LayerStack::popLayer() {
        std::shared_ptr<Layer> layer = layers.front();
        layers.pop_front();
        layer->onDetach();
        return layer;
    }

    std::shared_ptr<Layer> LayerStack::popOverLayer() {
        std::shared_ptr<Layer> layer = layers.back();
        layers.pop_back();
        layer->onDetach();
        return layer;
    }
}  // namespace Cube
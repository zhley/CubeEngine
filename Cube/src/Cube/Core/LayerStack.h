#pragma once

#include <memory>
#include <deque>

#include "Layer.h"

namespace Cube {
    class LayerStack {
    public:
        LayerStack() = default;
        ~LayerStack() = default;
        void pushLayer(const std::shared_ptr<Layer>& layer);
        void pushOverLayer(const std::shared_ptr<Layer>& layer);
        std::shared_ptr<Layer> popLayer();
        std::shared_ptr<Layer> popOverLayer();

        const std::deque<std::shared_ptr<Layer>>& getLayers() const { return layers; }
    private:
        std::deque<std::shared_ptr<Layer>> layers;
    };
}
#pragma once

namespace Cube {
    class Scene;

    class RenderServer {
    public:
        RenderServer() = default;
        ~RenderServer() = default;

        void renderScene(Scene* scene);
    };

}
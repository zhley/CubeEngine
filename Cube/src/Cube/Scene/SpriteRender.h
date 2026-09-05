#pragma once

#include "Component.h"
#include "Cube/Renderer/Color.h"
#include "Cube/Resource/ResPtr.h"
#include "Cube/Resource/Sprite.h"

namespace Cube {

    // Renders the node's sprite. Nodes holding this component are collected and
    // drawn by RenderServer during a tree render.
    class SpriteRender : public Component {
    public:
        ResPtr<Sprite> sprite;
        Color tintColor = {1.0f, 1.0f, 1.0f, 1.0f};
        int order = 0;

        SpriteRender() = default;
        ~SpriteRender() override = default;
        TypeID getType() const override { return getTypeID<SpriteRender>(); }
    };

}

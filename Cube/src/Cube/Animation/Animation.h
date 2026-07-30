#pragma once

#include <string>
#include <unordered_map>

#include "AnimationClip.h"
#include "Cube/Scene/Component.h"

namespace Cube {
    class SpriteRender;
    
    // Frame animation controller for sprites
    class Animation : public Component{
    public:
        std::unordered_map<std::string, ResPtr<AnimationClip>> clips;

        Animation() = default;
        ~Animation() override = default;

        void start() override;
        void update(float deltaTime) override;
        TypeID getType() const override { return getTypeID<Animation>(); }
        Sprite* getCurrentFrame();

        AnimationClip* addClip(const std::string& animClip);

        void play(const std::string& clipName);
        void stop();

    private:
        AnimationClip* currentClip = nullptr;
        bool playing = false;
        float currentTime = 0.0f;
        SpriteRender* spriteRender = nullptr;
    };

}  // namespace Cube
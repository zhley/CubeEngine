#pragma once

#include <string>
#include <vector>

#include "Cube/Core/Path.h"
#include "Cube/Resource/ResPtr.h"
#include "Cube/Resource/Sprite.h"

namespace Cube {

    struct AnimationFrame {
        ResPtr<Sprite> frame;
        float duration; // unit: second
    };

    class AnimationClip : public ResourceBase{
    public:
        AnimationClip(const Cube::Path& animFilePath);
        ~AnimationClip() = default;

        Sprite* getFrameAtTime(float time) const;

        const std::string& getName() const;
        bool isLooping() const;
        float getSpeed() const;
        float getDuration() const;

    private:
        std::vector<AnimationFrame> frames;
        std::string name;
        bool looping;
        float speed;
        float duration;
    };

}
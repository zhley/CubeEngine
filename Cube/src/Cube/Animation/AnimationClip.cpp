#include "AnimationClip.h"

#include <fstream>

#include "json.hpp"

#include "Cube/Resource/ResourceManager.h"

namespace Cube {
    // .anim
    AnimationClip::AnimationClip(const Cube::Path& animFilePath) {
        std::ifstream file(animFilePath.fspath());
        if(!file.is_open()) {
            CB_CORE_ERROR("Failed to open animation file {}", animFilePath);
            return;
        }
        nlohmann::json animData;
        file >> animData;
        file.close();
        name = animData["name"];
        looping = animData["looping"];
        speed = animData["speed"];
        duration = animData["duration"];
        for(auto& f : animData["frames"]) {
            AnimationFrame af;
            af.frame = ResPtr<Sprite>(f["frame"]);
            af.duration = f["duration"];
            frames.push_back(af);
        }
    }

    Sprite* AnimationClip::getFrameAtTime(float time) const {
        float accumulateTime = 0.0f;
        for(auto& frame : frames) {
            accumulateTime += frame.duration;
            if(accumulateTime >= time) {
                return frame.frame.get();
            }
        }
        return frames.back().frame.get();
    }

    const std::string& AnimationClip::getName() const { return name; }
    bool AnimationClip::isLooping() const { return looping; }
    float AnimationClip::getSpeed() const { return speed; }
    float AnimationClip::getDuration() const { return duration; }

}  // namespace Cube
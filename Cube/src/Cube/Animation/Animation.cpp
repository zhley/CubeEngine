#include "Animation.h"

#include "AnimationClip.h"
#include "Cube/Scene/Node.h"
#include "Cube/Scene/SpriteRender.h"

namespace Cube {

    void Animation::start() {
        spriteRender = node->getComponent<SpriteRender>();
        CB_ASSERT(spriteRender && "No SpriteRender component!");
    }

    void Animation::update(float deltaTime) {
        if(!playing || !currentClip) return;
        currentTime += deltaTime * currentClip->getSpeed();
        if(currentTime > currentClip->getDuration()) {
            if(currentClip->isLooping()) {
                currentTime -= currentClip->getDuration();
            }else {
                currentTime = currentClip->getDuration();
            }
        }
        spriteRender->sprite.reset(getCurrentFrame());
    }

    Sprite* Animation::getCurrentFrame() {
        if(currentClip) {
            return currentClip->getFrameAtTime(currentTime);
        }
        return nullptr;
    }

    AnimationClip* Animation::addClip(const std::string& animClip) {
        ResPtr<AnimationClip> clip(animClip);
        AnimationClip* clipPtr = clip.get();
        if(clip) {
            clips[clip->getName()] = std::move(clip);
            return clipPtr;
        }
        return nullptr;
    }

    void Animation::play(const std::string& clipName) {
        playing = true;
        currentTime = 0.0f;
        auto clip = clips.find(clipName);
        if(clip != clips.end()) {
            currentClip = clip->second.get();
        }else {
            CB_CORE_ERROR("Clip not found");
        }
    }

    void Animation::stop() {
        playing = false;
    }

}  // namespace Cube

#pragma once

#include "App/EditorApp.h"
#include "ResourcePickerDialog.h"
#include "View.h"

#include "Cube/Core/Path.h"
#include "Cube/Event/Event.h"
#include "Cube/Core/Engine.h"

#include <string>
#include <utility>
#include <vector>

class AnimationEditor : public View{
public:
    // Asks the animation editor to edit the animation clip resource identified by 'identifier'.
    class TargetChangeEvent : public Cube::Event {
    public:
        EVENT_TYPE(TargetChangeEvent)

        explicit TargetChangeEvent(std::string identifier) : identifier(std::move(identifier)) {}
        std::string identifier;
    };

    // Asks the animation editor to create a new animation clip resource.
    class NewClipEvent : public Cube::Event {
    public:
        EVENT_TYPE(NewClipEvent)
    };

    AnimationEditor(EditorPage& editorPage) : View(editorPage) {
        Cube::Engine::getApp()->getEventDispatcher().subscribe<TargetChangeEvent>(std::bind(&AnimationEditor::onTargetChange, this, std::placeholders::_1));
        Cube::Engine::getApp()->getEventDispatcher().subscribe<NewClipEvent>(std::bind(&AnimationEditor::onNewClip, this, std::placeholders::_1));
    }
    ~AnimationEditor() override = default;

    void render(float deltaTime) override;

private:
    struct FrameViewData {
        std::string frame;
        float duration = 0.0f;
    };

    bool loadTargetAnim();
    bool saveTargetAnim();
    bool createNewAnimationClip();
    void closeTargetAnim();

    bool onTargetChange(const Cube::Event& e);
    bool onNewClip(const Cube::Event& e);

    Cube::Path target;
    std::string targetIdentifier;
    std::string name;
    bool looping = false;
    float speed = 1.0f;
    float duration = 0.0f;
    bool dirty = false;
    std::vector<FrameViewData> frames;
    int selectedFrameIndex = -1;
    bool isPreviewPlaying = false;
    float previewTime = 0.0f;

    ResourcePickerDialog framePickerDialog;
};

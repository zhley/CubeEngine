#pragma once

#include "App/EditorApp.h"
#include "ResourcePickerDialog.h"
#include "View.h"

#include "Cube/Core/Path.h"
#include "Cube/Event/Event.h"
#include "Cube/Core/Engine.h"

#include <string>
#include <vector>

class AnimationEditor : public View{
public:
    class TargetChangeEvent : public Cube::Event {
    public:
        EVENT_TYPE(TargetChangeEvent)

        TargetChangeEvent(const Cube::Path& targetFilePath) : targetFilePath(targetFilePath) {}
        Cube::Path targetFilePath;
    };

    AnimationEditor(EditorPage& editorPage) : View(editorPage) {
        Cube::Engine::getApp()->getEventDispatcher().subscribe<TargetChangeEvent>(std::bind(&AnimationEditor::onTargetChange, this, std::placeholders::_1));
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
    bool createNewAnimationClip(const std::string& fileName);

    bool onTargetChange(const Cube::Event& e);

    Cube::Path target;
    std::string name;
    bool looping = false;
    float speed = 1.0f;
    float duration = 0.0f;
    std::vector<FrameViewData> frames;
    int selectedFrameIndex = -1;
    bool isPreviewPlaying = false;
    float previewTime = 0.0f;

    ResourcePickerDialog framePickerDialog;
};
#pragma once

#include <Windows.h>

#include "imgui/imgui.h"
#include "Cube/Core/Path.h"
#include "Cube/Renderer/FrameBuffer.h"

#include "View.h"

class SceneView : public View {
public:
    SceneView(EditorPage& editorPage);
    ~SceneView() override;
    void render(float deltaTime) override;

private:
    ImVec2 sceneViewSize = {800, 600};
    Cube::FrameBuffer* frameBuffer = nullptr;
    PROCESS_INFORMATION gameProcess = {};
    bool gameRunning = false;
    bool runConfirmOpen = false;

    void worldRender(float deltaTime);
    Cube::Path currentNodePath() const;
    std::string currentNodeResourceId() const;
    Cube::Path ensureGameExecutable() const;
    void startGameProcess(const Cube::Path& exePath, const std::string& nodeResourceId);
    void stopGameProcess();
    void runGame();
    void openRunConfirm();
    void renderRunConfirm();
    void markDirty();
};

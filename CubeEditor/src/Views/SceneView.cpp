#include "SceneView.h"

#include <algorithm>
#include <filesystem>
#include <string>
#include <vector>

#include "imgui/imgui.h"
#include "glm/glm.hpp"
#include "Cube/Animation/AnimationClip.h"
#include "Cube/Core/Log.h"
#include "Cube/Renderer/Renderer.h"
#include "Cube/Resource/NodeTree.h"
#include "Cube/Scene/Camera2D.h"
#include "Cube/Scene/Node.h"
#include "Cube/Scene/SpriteRender.h"
#include "Cube/Animation/Animation.h"
#include "Cube/Utils/Utils.h"

#include "../App/EditorPage.h"
#include "../Project/Project.h"
#include "../Utils/ImGuiExternal.h"
#include "../Utils/EditorTextureCache.h"
#include "../Utils/misc.h"
#include "Scene/EditorNodeAccess.h"

namespace {

// TODO: 改为用户在设置中配置的路径, 并在重新构建后支持覆盖更新
constexpr const char* kGameExeSourcePath = "D:/mycode/vsProject/CubeEngine/build/vscode/bin/CubeGame.exe";
constexpr const char* kGameExeName = "CubeGame.exe";

std::vector<Cube::Node*> collectRenderableNodes(Cube::Node* root) {
    std::vector<Cube::Node*> nodes;
    if(!root) {
        return nodes;
    }
    root->forEachNode([&nodes](Cube::Node* node) {
        Cube::SpriteRender* sprite = node->getComponent<Cube::SpriteRender>();
        if(sprite && sprite->sprite) {
            nodes.push_back(node);
        }
        return true;
    });
    std::sort(nodes.begin(), nodes.end(), [](Cube::Node* a, Cube::Node* b) {
        Cube::SpriteRender* sa = a->getComponent<Cube::SpriteRender>();
        Cube::SpriteRender* sb = b->getComponent<Cube::SpriteRender>();
        if(sa->order != sb->order) {
            return sa->order < sb->order;
        }
        return (sa->sprite->getTexture() ? sa->sprite->getTexture()->getId() : -1)
            < (sb->sprite->getTexture() ? sb->sprite->getTexture()->getId() : -1);
    });
    return nodes;
}

}  // namespace

SceneView::SceneView(EditorPage& editorPage) : View(editorPage) {
    frameBuffer = new Cube::FrameBuffer();
    frameBuffer->bindAttachment((int)sceneViewSize.x, (int)sceneViewSize.y);
}

SceneView::~SceneView() {
    stopGameProcess();
    delete frameBuffer;
}

void SceneView::markDirty() {
    editorPage.documentManager.getActive()->markDirty();
}

void SceneView::render(float deltaTime) {
    Cube::Texture2D* play_png = EditorTextureCache::get().request("assets/icons/play.png");

    ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoFocusOnAppearing);

    ImGui::BeginChild("ToolBar", {ImGui::GetWindowWidth(), 45});
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImVec2 toolButtonSize(37, 37);
    if(ImGui::ImageButton("play", play_png->getId(), toolButtonSize, ImVec2(0, 1), ImVec2(1, 0))) {
        openRunConfirm();
    }
    ImGui::SameLine();
    if(ImGui::Button("Reset")) {
        editorPage.editorCamera.position = {0, 0};
        editorPage.editorCamera.zoom = 1.0f;
        editorPage.editorCamera.viewport = toGlmVec2(sceneViewSize);
    }
    ImGui::PopStyleColor();
    ImGui::EndChild();

    if(ImGui::BeginTabBar("WorldTabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs)) {
        // Snapshot pointers: close removes the document from Project.
        std::vector<NodeDocument*> docs;
        for(const auto& doc : editorPage.documentManager.getDocuments()) {
            docs.push_back(doc.get());
        }
        for(NodeDocument* doc : docs) {
            ImGui::PushID(doc);
            const std::string label = doc->getRoot()->getName();
            std::string tabLabel = label + (doc->isDirty() ? "*" : "") + "###doc";
            bool open = true;
            const bool selected = ImGui::BeginTabItem(tabLabel.c_str(), &open);
            if(selected) {
                if(editorPage.documentManager.getActive() != doc) {
                    editorPage.documentManager.setActive(doc);
                }
                ImGui::BeginChild("World");
                ImVec2 currentSize = ImGui::GetContentRegionAvail();
                if(currentSize.x <= 0) currentSize.x = 1;
                if(currentSize.y <= 0) currentSize.y = 1;
                if((int)currentSize.x != (int)sceneViewSize.x || (int)currentSize.y != (int)sceneViewSize.y) {
                    sceneViewSize = currentSize;
                    editorPage.editorCamera.viewport = {sceneViewSize.x, sceneViewSize.y};
                    frameBuffer->resize((int)sceneViewSize.x, (int)sceneViewSize.y);
                }

                frameBuffer->bind();
                Cube::Renderer2D::setViewport((int)sceneViewSize.x, (int)sceneViewSize.y);
                Cube::Renderer2D::setClearColor(0.3f, 0.3f, 0.3f, 1.0f);
                Cube::Renderer2D::clearBuffer();
                worldRender(deltaTime);

                Cube::FrameBuffer::bindDefaultFrameBuffer();

                ImGui::Image(frameBuffer->getTexture(), sceneViewSize, ImVec2(0, 1), ImVec2(1, 0));
                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Asset")) {
                        AssetNode* asset = *(AssetNode**)payload->Data;
                        glm::vec2 pos = glm::vec2(ImGui::GetMousePos().x - ImGui::GetWindowPos().x, ImGui::GetWindowSize().y - (ImGui::GetMousePos().y - ImGui::GetWindowPos().y));
                        pos *= editorPage.editorCamera.zoom;
                        pos += editorPage.editorCamera.position;
                        switch(asset->type) {
                            case Cube::ResourceType::Texture: {
                                Cube::Node* n = EditorNodeAccess::addChild(*editorPage.documentManager.getActive()->getRoot(), asset->identifier);
                                n->pos = pos;
                                auto spriteRender = EditorNodeAccess::addComponent<Cube::SpriteRender>(*n);
                                spriteRender->sprite = Cube::ResPtr<Cube::Sprite>("spr:" + asset->identifier);
                                markDirty();
                            } break;
                            case Cube::ResourceType::AnimationClip: {
                                Cube::Node* n = EditorNodeAccess::addChild(*editorPage.documentManager.getActive()->getRoot(), asset->identifier);
                                n->pos = pos;
                                EditorNodeAccess::addComponent<Cube::SpriteRender>(*n);
                                auto anim = EditorNodeAccess::addComponent<Cube::Animation>(*n);
                                Cube::AnimationClip* clip = anim->addClip(asset->identifier);
                                if(clip) {
                                    anim->play(clip->getName());
                                }
                                markDirty();
                            } break;
                            default: break;
                        }
                    }
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetSprite")){
                        std::string spriteIdentifier((char*)payload->Data, payload->DataSize);
                        size_t posStr = spriteIdentifier.find('#');
                        if(posStr != std::string::npos) {
                            glm::vec2 pos = glm::vec2(ImGui::GetMousePos().x - ImGui::GetWindowPos().x, ImGui::GetWindowSize().y - (ImGui::GetMousePos().y - ImGui::GetWindowPos().y));
                            pos *= editorPage.editorCamera.zoom;
                            pos += editorPage.editorCamera.position;
                            std::string spriteName = spriteIdentifier.substr(posStr + 1);
                            Cube::Node* n = EditorNodeAccess::addChild(*editorPage.documentManager.getActive()->getRoot(), spriteName);
                            n->pos = pos;
                            auto spriteRender = EditorNodeAccess::addComponent<Cube::SpriteRender>(*n);
                            spriteRender->sprite = Cube::ResPtr<Cube::Sprite>(spriteIdentifier);
                            markDirty();
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
                if(ImGui::IsWindowFocused() && ImGui::IsWindowHovered()) {
                    EditorCamera& editorCamera = editorPage.editorCamera;
                    if(ImGui::IsKeyDown(ImGuiKey_LeftArrow)) {
                        editorCamera.position.x -= deltaTime * 500;
                    }
                    if(ImGui::IsKeyDown(ImGuiKey_RightArrow)) {
                        editorCamera.position.x += deltaTime * 500;
                    }
                    if(ImGui::IsKeyDown(ImGuiKey_UpArrow)) {
                        editorCamera.position.y += deltaTime * 500;
                    }
                    if(ImGui::IsKeyDown(ImGuiKey_DownArrow)) {
                        editorCamera.position.y -= deltaTime * 500;
                    }

                    static bool isPanning = false;
                    ImGuiIO& io = ImGui::GetIO();
                    if(ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
                        isPanning = true;
                    }
                    if(isPanning) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
                        glm::vec2 delta = {-io.MouseDelta.x * editorCamera.zoom, io.MouseDelta.y * editorCamera.zoom};
                        editorCamera.position += delta;

                        if(!ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
                            isPanning = false;
                        }
                    }

                    if(io.MouseWheel != 0.0f) {
                        // TODO: 设一个缩放界限
                        glm::vec2 mousePos = {io.MousePos.x - ImGui::GetWindowPos().x, ImGui::GetWindowSize().y - (io.MousePos.y - ImGui::GetWindowPos().y)};
                        glm::vec2 mouseWorldPos = mousePos * editorCamera.zoom + editorCamera.position;
                        static constexpr float E = 0.08f;
                        float k = std::pow(1.0f + E, io.MouseWheel);
                        editorCamera.zoom = editorCamera.zoom * k;
                        editorCamera.position = mouseWorldPos - mousePos * editorCamera.zoom;
                    }
                    static bool isDragging = false;
                    static bool isScaling = false;
                    if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered()) {
                        bool choose = false;
                        glm::vec2 mousePos = {io.MousePos.x - ImGui::GetWindowPos().x, ImGui::GetWindowSize().y - (io.MousePos.y - ImGui::GetWindowPos().y)};
                        glm::vec4 mouseWorldPos = editorCamera.getTransformMatrix() * glm::vec4(mousePos, 0.0f, 1.0f);
                        Cube::Node* selected = nullptr;
                        for(Cube::Node* n : collectRenderableNodes(editorPage.documentManager.getActive()->getRoot())) {
                            Cube::SpriteRender* sprite = n->getComponent<Cube::SpriteRender>();
                            if(!sprite || !sprite->sprite) continue;
                            glm::mat4 model = n->getWorldMatrix();
                            glm::mat4 corner = model * glm::mat4({
                                {0.0f, 0.0f, 0.0f, 1.0f},
                                {sprite->sprite->getSize().x, 0.0f, 0.0f, 1.0f},
                                {sprite->sprite->getSize().x, sprite->sprite->getSize().y, 0.0f, 1.0f},
                                {0.0f, sprite->sprite->getSize().y, 0.0f, 1.0f}
                            });
                            if(Utils::isPointInPolygon({mouseWorldPos.x, mouseWorldPos.y}, {{corner[0].x, corner[0].y}, {corner[1].x, corner[1].y}, {corner[2].x, corner[2].y}, {corner[3].x, corner[3].y}})) {
                                selected = n;
                            }
                        }
                        if(selected) {
                            if(editorPage.documentManager.getActive()->getSelectedNode() == selected) {
                                if(io.KeyShift) {
                                    isScaling = true;
                                    isDragging = false;
                                } else {
                                    isDragging = true;
                                    isScaling = false;
                                }
                            }
                            editorPage.documentManager.getActive()->selectNode(selected);
                            choose = true;
                        }
                        if(!choose) {
                            editorPage.documentManager.getActive()->selectNode(nullptr);
                            isDragging = false;
                            isScaling = false;
                        }
                    }
                    if(isDragging) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                        glm::vec2 delta = {io.MouseDelta.x * editorCamera.zoom, -io.MouseDelta.y * editorCamera.zoom};
                        if(editorPage.documentManager.getActive()->getSelectedNode()) {
                            editorPage.documentManager.getActive()->getSelectedNode()->pos = editorPage.documentManager.getActive()->getSelectedNode()->pos + delta;
                            markDirty();
                        }
                        if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                            isDragging = false;
                        }
                    }
                    if(isScaling) {
                        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                        if(editorPage.documentManager.getActive()->getSelectedNode()) {
                            glm::vec2 scale = editorPage.documentManager.getActive()->getSelectedNode()->scale;

                            const float scaleFactor = 1.0f + (io.MouseDelta.x - io.MouseDelta.y) * 0.01f;
                            const float safeScaleFactor = scaleFactor < 0.01f ? 0.01f : scaleFactor;

                            scale *= safeScaleFactor;
                            if(scale.x < 0.01f) scale.x = 0.01f;
                            if(scale.y < 0.01f) scale.y = 0.01f;

                            editorPage.documentManager.getActive()->getSelectedNode()->scale = scale;
                            markDirty();
                        }
                        if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                            isScaling = false;
                        }
                    }
                } // if focused/hovered
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            if(!open) {
                if(doc->isDirty()) {
                    pendingCloseDocument = doc;
                    closeConfirmOpen = true;
                } else {
                    editorPage.documentManager.close(doc);
                    ImGui::PopID();
                    break;
                }
            }
            ImGui::PopID();
        }
        ImGui::EndTabBar();
    }
    renderRunConfirm();
    renderCloseConfirm();
    ImGui::End();
}

void SceneView::worldRender(float deltaTime) {
    Cube::Node* root = editorPage.documentManager.getActive()->getRoot();

    const EditorCamera& editorCamera = editorPage.editorCamera;
    Cube::Renderer2D::beginFrame(editorCamera.getPVMatrix());
    // the axis lines
    float left = editorCamera.position.x;
    float right = editorCamera.position.x + editorCamera.viewport.x * editorCamera.zoom;
    float bottom = editorCamera.position.y;
    float top = editorCamera.position.y + editorCamera.viewport.y * editorCamera.zoom;
    Cube::Renderer2D::drawLine({left, 0}, {right, 0}, {1.0f, 0.0f, 0.0f, 1.0f}, 1.0f * editorCamera.zoom);
    Cube::Renderer2D::drawLine({0, bottom}, {0, top}, {0.0f, 0.0f, 1.0f, 1.0f}, 1.0f * editorCamera.zoom);

    root->forEachNode([&editorCamera](Cube::Node* node) {
        Cube::Camera2D* cc = node->getComponent<Cube::Camera2D>();
        if(cc && cc->available) {
            glm::vec2 size = cc->viewport;
            Cube::Color color = {113, 96, 232, 255};
            Cube::Renderer2D::drawRect(node->getWorldMatrix(), size, color, 1.0f * editorCamera.zoom);
        }
        return true;
    });

    for(Cube::Node* n : collectRenderableNodes(root)) {
        auto* sc = n->getComponent<Cube::SpriteRender>();
        Cube::Renderer2D::drawQuad(n->getWorldMatrix(), sc->tintColor, sc->sprite->getTexture(), sc->sprite->getTexRegion().getUVCoord());
    }

    // the outline of selected node
    if(editorPage.documentManager.getActive()->getSelectedNode() && editorPage.documentManager.getActive()->getSelectedNode()->hasComponent<Cube::SpriteRender>() && editorPage.documentManager.getActive()->getSelectedNode()->getComponent<Cube::SpriteRender>()->sprite) {
        Cube::SpriteRender* sr = editorPage.documentManager.getActive()->getSelectedNode()->getComponent<Cube::SpriteRender>();
        glm::mat4 model = editorPage.documentManager.getActive()->getSelectedNode()->getWorldMatrix();
        glm::vec2 size = sr->sprite->getSize();
        Cube::Color color = {255, 255, 0, 255};
        Cube::Renderer2D::drawRect(model, size, color, 1.0f * editorCamera.zoom);
    }

    Cube::Renderer2D::endFrame();
}

Cube::Path SceneView::ensureGameExecutable() const {
    const Cube::Path target = editorPage.getProject()->getConfig().rootPath / kGameExeName;
    if(std::filesystem::exists(target.fspath())) {
        return target;
    }
    const Cube::Path source(kGameExeSourcePath);
    if(!std::filesystem::exists(source.fspath())) {
        CB_EDITOR_ERROR("SceneView: game executable not found at '{}'", source);
        return Cube::Path();
    }
    // TODO: 时间戳落后也要更新
    std::filesystem::copy_file(source.fspath(), target.fspath());
    CB_EDITOR_INFO("SceneView: copied game executable to '{}'", target);
    return target;
}

void SceneView::startGameProcess(const Cube::Path& exePath, const std::string& nodeResourceId) {
    stopGameProcess();

    const std::string commandLine = '"' + exePath.string() + "\" -n " + nodeResourceId;
    std::u16string commandLineUtf16 = Cube::Utils::utf8To16(commandLine);
    const std::u16string workingDirUtf16 = Cube::Utils::utf8To16(editorPage.getProject()->getConfig().rootPath.string());
    const std::wstring workingDir(workingDirUtf16.begin(), workingDirUtf16.end());

    STARTUPINFOW startupInfo = {};
    startupInfo.cb = sizeof(startupInfo);
    bool success = CreateProcessW(nullptr, reinterpret_cast<LPWSTR>(&commandLineUtf16[0]), nullptr, nullptr, FALSE, 0, nullptr, workingDir.c_str(), &startupInfo, &gameProcess);
    if(!success) {
        CB_EDITOR_ERROR("SceneView: failed to start game process '{}'", exePath);
        return;
    }
    gameRunning = true;
    CB_EDITOR_INFO("SceneView: game started, node = '{}'", nodeResourceId);
}

void SceneView::stopGameProcess() {
    if(!gameRunning) {
        return;
    }
    gameRunning = false;
    if(WaitForSingleObject(gameProcess.hProcess, 0) != WAIT_OBJECT_0) {
        TerminateProcess(gameProcess.hProcess, 0);
        CB_EDITOR_INFO("SceneView: game process terminated");
    }
    CloseHandle(gameProcess.hProcess);
    CloseHandle(gameProcess.hThread);
    gameProcess = {};
}

void SceneView::runGame() {
    if(!editorPage.documentManager.getActive() || !editorPage.documentManager.getActive()->getRoot()) {
        CB_EDITOR_ERROR("SceneView: no node document selected, cannot run the game");
        return;
    }
    editorPage.getProject()->save();
    NodeDocument* activeDoc = editorPage.documentManager.getActive();
    startGameProcess(ensureGameExecutable(), activeDoc->getIdentifier());
}

void SceneView::openRunConfirm() {
    if(!editorPage.documentManager.getActive() || !editorPage.documentManager.getActive()->getRoot()) {
        CB_EDITOR_ERROR("SceneView: no node document selected, cannot run the game");
        return;
    }
    if(!editorPage.documentManager.getActive()->isDirty()) {
        runGame();
        return;
    }
    runConfirmOpen = true;
}

void SceneView::renderRunConfirm() {
    if(!runConfirmOpen) {
        return;
    }
    ImGui::OpenPopup("Run Game##SceneView");
    if(ImGui::BeginPopupModalSuper("Run Game##SceneView", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("The current node tree has unsaved changes.");
        ImGui::Spacing();
        if(ImGui::Button("Save and Run")) {
            editorPage.documentManager.save(editorPage.documentManager.getActive());
            editorPage.documentManager.getActive()->markSaved();
            runConfirmOpen = false;
            ImGui::CloseCurrentPopup();
            runGame();
        }
        ImGui::SameLine();
        if(ImGui::Button("Run Without Saving")) {
            runConfirmOpen = false;
            ImGui::CloseCurrentPopup();
            runGame();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel")) {
            runConfirmOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SceneView::renderCloseConfirm() {
    if(!closeConfirmOpen) {
        return;
    }
    ImGui::OpenPopup("Close Document##SceneView");
    if(ImGui::BeginPopupModalSuper("Close Document##SceneView", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("The node tree has unsaved changes.");
        ImGui::Spacing();

        bool closePopup = false;
        if(ImGui::Button("Save")) {
            NodeDocument* document = pendingCloseDocument;
            editorPage.documentManager.save(document);
            // The document stays open if writing the file failed.
            if(document && !document->isDirty()) {
                editorPage.documentManager.close(document);
            }
            closePopup = true;
        }
        ImGui::SameLine();
        if(ImGui::Button("Don't Save")) {
            editorPage.documentManager.close(pendingCloseDocument);
            closePopup = true;
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel")) {
            closePopup = true;
        }
        if(closePopup) {
            pendingCloseDocument = nullptr;
            closeConfirmOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

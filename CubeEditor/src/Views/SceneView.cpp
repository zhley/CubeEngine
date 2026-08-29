#include "SceneView.h"

#include <imgui/imgui.h>

#include <glm/ext/matrix_clip_space.hpp>
#include <thread>

#include "../App/EditorPage.h"
#include "../App/EditorApp.h"
#include "../Game.h"
#include "../Project/Project.h"
#include "../Utils/ImGuiExternal.h"
#include "../Utils/EditorTextureCache.h"
#include "../Utils/misc.h"
#include "Cube/Animation/AnimationClip.h"
#include "Cube/Renderer/Renderer.h"
#include "Cube/Scene/Camera2D.h"
#include "Cube/Scene/SpriteRender.h"
#include "Cube/Animation/Animation.h"
#include "glm/ext/vector_float4.hpp"

SceneView::SceneView(EditorPage& editorPage) : View(editorPage) {
    frameBuffer = new Cube::FrameBuffer();
    frameBuffer->bindAttachment((int)sceneViewSize.x, (int)sceneViewSize.y);
}

SceneView::~SceneView() {
    delete frameBuffer;
}

void SceneView::render(float deltaTime) {
    Cube::Texture2D* play_png = EditorTextureCache::get().request("assets/icons/play.png");

    ImGui::Begin("Scene View");
    
    ImGui::BeginChild("ToolBar", {ImGui::GetWindowWidth(), 45});
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
    ImVec2 toolButtonSize(37, 37);
    EditorApp* app = static_cast<EditorApp*>(Cube::Engine::getApp());
    static bool isGameOver = true;
    if(ImGui::ImageButton("play", play_png->getId(), toolButtonSize, ImVec2(0, 1), ImVec2(1, 0)) && isGameOver) {
    }
    ImGui::SameLine();
    if(ImGui::Button("Reset")) {
        editorPage.editorCamera.position = {0, 0};
        editorPage.editorCamera.zoom = 1.0f;
        editorPage.editorCamera.viewport = toGlmVec2(sceneViewSize);
    }
    ImGui::PopStyleColor();
    ImGui::EndChild();
    
    ImGui::BeginChild("Scene");
    if(editorPage.selectedScene) {
        ImVec2 currentSize = ImGui::GetContentRegionAvail();
        if(currentSize.x <= 0) currentSize.x = 1;
        if(currentSize.y <= 0) currentSize.y = 1;
        if((int)currentSize.x != (int)sceneViewSize.x || (int)currentSize.y != (int)sceneViewSize.y) {
            sceneViewSize = currentSize;
            editorPage.editorCamera.viewport = {sceneViewSize.x, sceneViewSize.y};
            frameBuffer->resize((int)sceneViewSize.x, (int)sceneViewSize.y);
        }

        editorPage.selectedScene->scene->update(deltaTime);
    
        frameBuffer->bind();
        Cube::Renderer2D::setViewport((int)sceneViewSize.x, (int)sceneViewSize.y);
        Cube::Renderer2D::setClearColor(0.3f, 0.3f, 0.3f, 1.0f);
        Cube::Renderer2D::clearBuffer();
        // scene render
        sceneRender(deltaTime);
    
        Cube::FrameBuffer::bindDefaultFrameBuffer();
    
        static bool showSelectSubTexturePopup = false;
        ImGui::Image(frameBuffer->getTexture(), sceneViewSize, ImVec2(0, 1), ImVec2(1, 0));
        if(ImGui::BeginDragDropTarget()) {
            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Asset")) {
                AssetNode* asset = *(AssetNode**)payload->Data;
                glm::vec2 pos = glm::vec2(ImGui::GetMousePos().x - ImGui::GetWindowPos().x, ImGui::GetWindowSize().y - (ImGui::GetMousePos().y - ImGui::GetWindowPos().y));
                pos *= editorPage.editorCamera.zoom;
                pos += editorPage.editorCamera.position;
                switch(asset->type) {
                    case Cube::ResourceType::Texture: {
                        auto e = editorPage.selectedScene->scene->createEntity(asset->identifier);
                        e->getTransform().pos = pos;
                        auto spriteRender = e->addComponent<Cube::SpriteRender>();
                        spriteRender->sprite = Cube::ResPtr<Cube::Sprite>("spr:" + asset->identifier);
                        editorPage.selectedScene->isSaved = false;
                    } break;
                    case Cube::ResourceType::AnimationClip: {
                        auto e = editorPage.selectedScene->scene->createEntity(asset->identifier);
                        e->getTransform().pos = pos;
                        e->addComponent<Cube::SpriteRender>();
                        auto anim = e->addComponent<Cube::Animation>();
                        Cube::AnimationClip* clip = anim->addClip(asset->identifier);
                        if(clip) {
                            anim->play(clip->getName());
                        }
                        editorPage.selectedScene->isSaved = false;
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
                    auto e = editorPage.selectedScene->scene->createEntity(spriteName);
                    e->getTransform().pos = pos;
                    auto spriteRender = e->addComponent<Cube::SpriteRender>();
                    spriteRender->sprite = Cube::ResPtr<Cube::Sprite>(spriteIdentifier);
                    editorPage.selectedScene->isSaved = false;
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
                Cube::Entity* selected = nullptr;
                for(auto& e : editorPage.selectedScene->scene->getSortedRenderableEntities()) {
                    Cube::Transform& tc = e->getTransform();
                    Cube::SpriteRender* sprite = e->getComponent<Cube::SpriteRender>();
                    if(!sprite || !sprite->sprite) continue;
                    glm::mat4 model = tc.getWorldMatrix();
                    glm::mat4 corner = model * glm::mat4({
                        {0.0f, 0.0f, 0.0f, 1.0f},
                        {sprite->sprite->getSize().x, 0.0f, 0.0f, 1.0f},
                        {sprite->sprite->getSize().x, sprite->sprite->getSize().y, 0.0f, 1.0f},
                        {0.0f, sprite->sprite->getSize().y, 0.0f, 1.0f}
                    });
                    if(Utils::isPointInPolygon({mouseWorldPos.x, mouseWorldPos.y}, {{corner[0].x, corner[0].y}, {corner[1].x, corner[1].y}, {corner[2].x, corner[2].y}, {corner[3].x, corner[3].y}})) {
                        selected = e;
                    }
                }
                if(selected) {
                    if(editorPage.selectedEntity == selected) {
                        if(io.KeyShift) {
                            isScaling = true;
                            isDragging = false;
                        } else {
                            isDragging = true;
                            isScaling = false;
                        }
                    }
                    editorPage.selectedEntity = selected;
                    choose = true;
                }
                if(!choose) {
                    editorPage.selectedEntity = nullptr;
                    isDragging = false;
                    isScaling = false;
                }
            }
            if(isDragging) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                glm::vec2 delta = {io.MouseDelta.x * editorCamera.zoom, -io.MouseDelta.y * editorCamera.zoom};
                if(editorPage.selectedEntity) {
                    Cube::Transform& tc = editorPage.selectedEntity->getTransform();
                    tc.pos = tc.pos + delta;
                    editorPage.selectedScene->isSaved = false;
                }
                if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    isDragging = false;
                }
            }
            if(isScaling) {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
                if(editorPage.selectedEntity) {
                    Cube::Transform& tc = editorPage.selectedEntity->getTransform();
                    glm::vec2 scale = tc.scale;

                    const float scaleFactor = 1.0f + (io.MouseDelta.x - io.MouseDelta.y) * 0.01f;
                    const float safeScaleFactor = scaleFactor < 0.01f ? 0.01f : scaleFactor;

                    scale *= safeScaleFactor;
                    if(scale.x < 0.01f) scale.x = 0.01f;
                    if(scale.y < 0.01f) scale.y = 0.01f;

                    tc.scale = scale;
                    editorPage.selectedScene->isSaved = false;
                }
                if(!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    isScaling = false;
                }
            }
        }
    }
    ImGui::EndChild();
    ImGui::End();
}

void SceneView::sceneRender(float deltaTime) {
    Cube::Scene* scene = editorPage.selectedScene->scene;
    
    const EditorCamera& editorCamera = editorPage.editorCamera;
    Cube::Renderer2D::beginFrame(editorCamera.getPVMatrix());
    // the axis lines
    float left = editorCamera.position.x;
    float right = editorCamera.position.x + editorCamera.viewport.x * editorCamera.zoom;
    float bottom = editorCamera.position.y;
    float top = editorCamera.position.y + editorCamera.viewport.y * editorCamera.zoom;
    Cube::Renderer2D::drawLine({left, 0}, {right, 0}, {1.0f, 0.0f, 0.0f, 1.0f}, 1.0f * editorCamera.zoom);
    Cube::Renderer2D::drawLine({0, bottom}, {0, top}, {0.0f, 0.0f, 1.0f, 1.0f}, 1.0f * editorCamera.zoom);

    for(auto& camera : scene->getCameras()) {
        auto* tc = &camera->getTransform();
        auto* cc = camera->getComponent<Cube::Camera2D>();
        if(cc->available) {
            glm::vec2 size = cc->viewport;
            Cube::Color color = {113, 96, 232, 255};
            Cube::Renderer2D::drawRect(tc->getWorldMatrix(), size, color, 1.0f * editorCamera.zoom);
        }
    }

    for(auto& e : scene->getSortedRenderableEntities()) {
        auto* sc = e->getComponent<Cube::SpriteRender>();
        if(sc->sprite){
            Cube::Renderer2D::drawQuad(e->getTransform().getWorldMatrix(), sc->tintColor, sc->sprite->getTexture(), sc->sprite->getTexRegion().getUVCoord());
        }
    }

    // the outline of selected entity
    if(editorPage.selectedEntity && editorPage.selectedEntity->hasComponent<Cube::SpriteRender>() && editorPage.selectedEntity->getComponent<Cube::SpriteRender>()->sprite) {
        Cube::SpriteRender* sr = editorPage.selectedEntity->getComponent<Cube::SpriteRender>();
        glm::mat4 model = editorPage.selectedEntity->getTransform().getWorldMatrix();
        glm::vec2 size = sr->sprite->getSize();
        Cube::Color color = {255, 255, 0, 255};
        Cube::Renderer2D::drawRect(model, size, color, 1.0f * editorCamera.zoom);
    }

    Cube::Renderer2D::endFrame();
}
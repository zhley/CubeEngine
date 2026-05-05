#include "EntityPropertyPanel.h"
#include <unordered_map>
#include <vector>

#include "../App/EditorPage.h"
#include "../Project/Project.h"
#include "../Utils/ImGuiExternal.h"
#include "Cube/Animation/Animation.h"
#include "Cube/Core/Log.h"
#include "Cube/Resource/ResourceManager.h"
#include "Cube/Resource/Sprite.h"
#include "Cube/Scene/Camera2D.h"
#include "Cube/Scene/Component.h"
#include "Cube/Scene/SpriteRender.h"
#include "SceneView.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

using namespace Cube;

void EntityPropertyPanel::render(float deltaTime) {
    ImGui::Begin("Entity Properties");

    if(editorPage.selectedEntity) {
        float posX = ImGui::GetWindowWidth() / 2 - 30.0f;
        float width = ImGui::GetWindowWidth() - posX - 10.0f;
        if(ImGui::TreeNodeEx("Transform", Utils::TREENODE_FLAGS)) {
            Transform& transform = editorPage.selectedEntity->getTransform();
            ImGui::Text("position");
            ImGui::SameLine();
            ImGui::SetCursorPosX(posX);
            ImGui::SetNextItemWidth(width);
            float pos[2] = {transform.pos.x, transform.pos.y};
            if(ImGui::DragFloat2("##position", pos, 0.1, 0, 0, "%.3f")) {
                editorPage.selectedScene->isSaved = false;
                transform.pos = {pos[0], pos[1]};
            }

            ImGui::Text("scale");
            float scale[2] = {transform.scale.x, transform.scale.y};
            ImGui::SameLine();
            ImGui::SetCursorPosX(posX);
            ImGui::SetNextItemWidth(width);
            if(ImGui::DragFloat2("##Scale", scale, 0.01, 0, 0, "%.3f")){
                editorPage.selectedScene->isSaved = false;
                transform.scale = {scale[0], scale[1]};
            }
            
            ImGui::Text("rotation");
            ImGui::SameLine();
            ImGui::SetCursorPosX(posX);
            ImGui::SetNextItemWidth(width);
            float rotation = transform.rotation;
            if(ImGui::DragFloat("##Rotation", &rotation, 0.1, 0, 0, "%.3f")) {
                editorPage.selectedScene->isSaved = false;
                transform.rotation = rotation;
            }

            ImGui::TreePop();
        }
        TypeID toDelete = 0;
        for(auto& uniquePtrC : editorPage.selectedEntity->getComponents()) {
            TypeID typeID = uniquePtrC->getType();
            Component* c = uniquePtrC.get();
            Class* classInfo = ClassRegistry::get().getClass(typeID);
            if(ImGui::TreeNodeEx(classInfo->getName().c_str(), Utils::TREENODE_FLAGS)) {
                if(ImGui::BeginPopupContextItem()) {
                    if(ImGui::MenuItem("Delete")) {
                        toDelete = typeID;
                    }
                    ImGui::EndPopup();
                }
                for(auto& property : classInfo->getAllProperties()) {
                    ImGui::Text(property->getName().c_str());
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(posX);
                    ImGui::SetNextItemWidth(width);
                    if(property->getTypeID() == getTypeID<float>()) {
                        float v = property->getValue(c).as<float>();
                        if(ImGui::DragFloat(("##" + property->getName()).c_str(), &v, 0.01, 0, 0, "%.3f")) {
                            editorPage.selectedScene->isSaved = false;
                            property->setValue(c, v);
                        }
                    } else if(property->getTypeID() == getTypeID<double>()) {
                        float v = (float)property->getValue(c).as<double>();
                        if(ImGui::DragFloat(("##" + property->getName()).c_str(), &v, 0.01, 0, 0, "%.3f")) {
                            editorPage.selectedScene->isSaved = false;
                            property->setValue(c, v);
                        }
                    } else if(property->getTypeID() == getTypeID<glm::vec2>()) {
                        glm::vec2 v = property->getValue(c).as<glm::vec2>();
                        float v2[2] = {v.x, v.y};
                        if(ImGui::DragFloat2(("##" + property->getName()).c_str(), v2, 0.01, 0, 0, "%.3f")) {
                            editorPage.selectedScene->isSaved = false;
                            property->setValue(c, glm::vec2(v2[0], v2[1]));
                        }
                    } else if(property->getTypeID() == getTypeID<glm::vec3>()) {
                        glm::vec3 v = property->getValue(c).as<glm::vec3>();
                        float v3[3] = {v.x, v.y, v.z};
                        if(ImGui::DragFloat3(("##" + property->getName()).c_str(), v3, 0.01, 0, 0, "%.3f")) {
                            editorPage.selectedScene->isSaved = false;
                            property->setValue(c, glm::vec3(v3[0], v3[1], v3[2]));
                        }
                    } else if(property->getTypeID() == getTypeID<glm::vec4>()) {
                        glm::vec4 v = property->getValue(c).as<glm::vec4>();
                        float v4[4] = {v.x, v.y, v.z, v.w};
                        if(ImGui::DragFloat4(("##" + property->getName()).c_str(), v4, 0.01, 0, 0, "%.3f")) {
                            editorPage.selectedScene->isSaved = false;
                            property->setValue(c, glm::vec4(v4[0], v4[1], v4[2], v4[3]));
                        }
                    } else if(property->getTypeID() == getTypeID<bool>()) {
                        bool v = property->getValue(c).as<bool>();
                        if(ImGui::Checkbox(("##" + property->getName()).c_str(), &v)) {
                            editorPage.selectedScene->isSaved = false;
                            property->setValue(c, v);
                        }
                    } else if(property->getTypeID() == getTypeID<int>()) {
                        int v = property->getValue(c).as<int>();
                        if(ImGui::DragInt(("##" + property->getName()).c_str(), &v, 1, 0, 0)) {
                            editorPage.selectedScene->isSaved = false;
                            property->setValue(c, v);
                        }
                    } else if(property->getTypeID() == getTypeID<std::string>()) {
                        char buffer[256] = {};
                        strcpy_s(buffer, property->getValue(c).as<std::string>().c_str());
                        if(ImGui::InputText(("##" + property->getName()).c_str(), buffer, IM_ARRAYSIZE(buffer))) {
                            editorPage.selectedScene->isSaved = false;
                            property->setValue(c, std::string(buffer));
                        }
                    } else if(property->getTypeID() == getTypeID<Color>()) {
                        Color color = property->getValue(c).as<Color>();
                        float colorV[4] = {color.r, color.g, color.b, color.a};
                        if(ImGui::ColorEdit4(("##" + property->getName()).c_str(), colorV, ImGuiColorEditFlags_DisplayHex)) {
                            editorPage.selectedScene->isSaved = false;
                            property->setValue(c, Color(colorV[0], colorV[1], colorV[2], colorV[3]));
                        }
                    } else if(property->getTypeID() == getTypeID<TextureRegion>()) {
                        glm::vec4 v = property->getValue(c).as<TextureRegion>().getUVCoord();
                        float v4[4] = {v.x, v.y, v.z, v.w};
                        if(ImGui::DragFloat4(("##" + property->getName()).c_str(), v4, 0.001f, 0, 1)) {
                            editorPage.selectedScene->isSaved = false;
                            TextureRegion tr;
                            tr.uvMin = {v4[0], v4[1]};
                            tr.uvMax = {v4[2], v4[3]};
                            property->setValue(c, tr);
                        }
                    } else if(property->getTypeID() == getTypeID<ResPtr<Sprite>>()) {
                        ResPtr<Sprite> res = property->getValue(c).as<ResPtr<Sprite>>();
                        if(res) {
                            ImVec2 size = Utils::keepAspectRatio(toImVec2(res->getSize()), 100.0f);
                            ImGui::Image(res->getTexture()->getId(), size, {res->getTexRegion().uvMin.x, res->getTexRegion().uvMax.y}, {res->getTexRegion().uvMax.x, res->getTexRegion().uvMin.y});
                        }else {
                            ImGui::Text("None");
                        }
                        if(ImGui::BeginDragDropTarget()) {
                            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Asset")) {
                                AssetNode* asset = *(AssetNode**)payload->Data;
                                if(asset->type == ResourceType::Texture) {
                                    property->setValue(c, ResPtr<Sprite>("spr:" + asset->identifier));
                                    editorPage.selectedScene->isSaved = false;
                                }
                            }
                            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetSprite")){
                                std::string spriteIdentifier((char*)payload->Data, payload->DataSize);
                                property->setValue(c, ResPtr<Sprite>(spriteIdentifier));
                                editorPage.selectedScene->isSaved = false;
                            }
                            ImGui::EndDragDropTarget();
                        }
                    } else if (property->getTypeID() == getTypeID<std::unordered_map<std::string, ResPtr<AnimationClip>>>()) {
                        auto animClips = property->getValue(c).as<std::unordered_map<std::string, ResPtr<AnimationClip>>>();
                        ImGui::BeginTable("AnimClips", 2);
                        for (auto& [name, clip] : animClips) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text(name.c_str());
                            ImGui::TableNextColumn();
                            if (clip) {
                                ImVec2 size = Utils::keepAspectRatio(toImVec2(clip->getFrameAtTime(0)->getSize()), 100.0f);
                                ImGui::Image(clip->getFrameAtTime(0)->getTexture()->getId(), size, {clip->getFrameAtTime(0)->getTexRegion().uvMin.x, clip->getFrameAtTime(0)->getTexRegion().uvMax.y}, {clip->getFrameAtTime(0)->getTexRegion().uvMax.x, clip->getFrameAtTime(0)->getTexRegion().uvMin.y});
                            } else {
                                ImGui::Text("None");
                            }
                        }
                        ImGui::EndTable();
                        if(ImGui::Button("Add Clip")){
                            resourcePickerDialog.open("Select Animation Clip", editorPage.getProject()->getAssetExplorer().getRootNode(), ResourceType::AnimationClip);
                        }
                        if(ImGui::BeginDragDropTarget()) {
                            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Asset")) {
                                AssetNode* asset = *(AssetNode**)payload->Data;
                                if(asset->type == ResourceType::AnimationClip) {
                                    animClips[asset->identifier] = ResPtr<AnimationClip>(asset->identifier);
                                    property->setValue(c, animClips);
                                    editorPage.selectedScene->isSaved = false;
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                        std::vector<std::string> pickedIdentifiers;
                        if(resourcePickerDialog.render(pickedIdentifiers, editorPage)) {
                            for(const auto& identifier : pickedIdentifiers) {
                                animClips[identifier] = ResPtr<AnimationClip>(identifier);
                            }
                            property->setValue(c, animClips);
                            editorPage.selectedScene->isSaved = false;
                        }
                    } else {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                        ImGui::Text("Failed to display property");
                        ImGui::PopStyleColor();
                    }
                }
                ImGui::TreePop();
            }
        }
        if(toDelete) {
            editorPage.selectedEntity->removeComponent(toDelete);
            editorPage.selectedScene->isSaved = false;
        }

        float w = ImGui::GetContentRegionAvail().x;
        static bool show = false;
        if(ImGui::Button("Add Component", {w, 0})) {
            ImGui::OpenPopup("addComponent");
        }
        if(ImGui::BeginPopup("addComponent")) {
            if(ImGui::MenuItem("SpriteRender")) {
                editorPage.selectedEntity->addComponent<SpriteRender>();
            }
            if(ImGui::MenuItem("Camera2D")) {
                editorPage.selectedEntity->addComponent<Camera2D>();
            }
            if(ImGui::MenuItem("Animation")) {
                editorPage.selectedEntity->addComponent<Animation>();
            }
            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

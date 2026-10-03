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
#include "Cube/Scene/Node.h"
#include "Cube/Scene/SpriteRender.h"
#include "Cube/Reflection/ClassRegistry.h"
#include "Scene/EditorNodeAccess.h"
#include "SceneView.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

void EntityPropertyPanel::render(float deltaTime) {
    ImGui::Begin("Node Properties");

    if(editorPage.documentManager.getActive() && editorPage.documentManager.getActive()->getSelectedNode()) {
        float posX = ImGui::GetWindowWidth() / 2 - 30.0f;
        float width = ImGui::GetWindowWidth() - posX - 10.0f;
        if(ImGui::TreeNodeEx("Transform", Utils::TREENODE_FLAGS)) {
            Cube::Node* node = editorPage.documentManager.getActive()->getSelectedNode();
            ImGui::Text("position");
            ImGui::SameLine();
            ImGui::SetCursorPosX(posX);
            ImGui::SetNextItemWidth(width);
            float pos[2] = {node->pos.x, node->pos.y};
            if(ImGui::DragFloat2("##position", pos, 0.1, 0, 0, "%.3f")) {
                editorPage.documentManager.getActive()->markDirty();
                node->pos = {pos[0], pos[1]};
            }

            ImGui::Text("scale");
            float scale[2] = {node->scale.x, node->scale.y};
            ImGui::SameLine();
            ImGui::SetCursorPosX(posX);
            ImGui::SetNextItemWidth(width);
            if(ImGui::DragFloat2("##Scale", scale, 0.01, 0, 0, "%.3f")){
                editorPage.documentManager.getActive()->markDirty();
                node->scale = {scale[0], scale[1]};
            }

            ImGui::Text("rotation");
            ImGui::SameLine();
            ImGui::SetCursorPosX(posX);
            ImGui::SetNextItemWidth(width);
            float rotation = node->rotation;
            if(ImGui::DragFloat("##Rotation", &rotation, 0.1, 0, 0, "%.3f")) {
                editorPage.documentManager.getActive()->markDirty();
                node->rotation = rotation;
            }

            ImGui::TreePop();
        }
        Cube::Node* selectedNode = editorPage.documentManager.getActive()->getSelectedNode();
        Cube::TypeID toDelete = 0;
        std::string scriptToDelete;
        for(Cube::Component* c : selectedNode->getComponents()) {
            Cube::TypeID typeID = c->getType();
            const bool isScript = typeID == Cube::getTypeID<Cube::ScriptComponent>();
            Cube::Class* classInfo = Cube::ClassRegistry::get().getClass(typeID);
            const std::string title = isScript ? ("Script: " + static_cast<Cube::ScriptComponent*>(c)->getName()) : classInfo->getName();
            if(ImGui::TreeNodeEx(title.c_str(), Utils::TREENODE_FLAGS)) {
                if(ImGui::BeginPopupContextItem()) {
                    if(ImGui::MenuItem("Delete")) {
                        if(isScript) {
                            scriptToDelete = static_cast<Cube::ScriptComponent*>(c)->getName();
                        } else {
                            toDelete = typeID;
                        }
                    }
                    ImGui::EndPopup();
                }
                if(isScript) {
                    Cube::ScriptComponent* scriptComp = static_cast<Cube::ScriptComponent*>(c);
                    ImGui::Text("script");
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(posX);
                    ImGui::TextWrapped("%s", scriptComp->getScript() ? scriptComp->getScript()->getIdentifier().c_str() : "<none>");
                    if(!scriptComp->getInstance()) {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
                        ImGui::TextUnformatted("Script instance is unavailable");
                        ImGui::PopStyleColor();
                    } else {
                        nlohmann::json instanceData = scriptComp->serializeInstance();
                        bool instanceChanged = false;
                        for(auto& [key, field] : instanceData.items()) {
                            if(!field.contains("type") || !field.contains("value")) {
                                continue;
                            }
                            const std::string fieldType = field["type"].get<std::string>();
                            ImGui::Text("%s", key.c_str());
                            ImGui::SameLine();
                            ImGui::SetCursorPosX(posX);
                            ImGui::SetNextItemWidth(width);
                            const std::string widgetId = "##" + key;
                            if(fieldType == "Int") {
                                int v = static_cast<int>(field["value"].get<int64_t>());
                                if(ImGui::DragInt(widgetId.c_str(), &v)) {
                                    field["value"] = static_cast<int64_t>(v);
                                    instanceChanged = true;
                                }
                            } else if(fieldType == "Float") {
                                float v = field["value"].get<float>();
                                if(ImGui::DragFloat(widgetId.c_str(), &v, 0.01f, 0.0f, 0.0f, "%.3f")) {
                                    field["value"] = v;
                                    instanceChanged = true;
                                }
                            } else if(fieldType == "Bool") {
                                bool v = field["value"].get<bool>();
                                if(ImGui::Checkbox(widgetId.c_str(), &v)) {
                                    field["value"] = v;
                                    instanceChanged = true;
                                }
                            } else if(fieldType == "String" || fieldType == "StrObj") {
                                char buffer[256] = {};
                                strcpy_s(buffer, field["value"].get<std::string>().c_str());
                                if(ImGui::InputText(widgetId.c_str(), buffer, IM_ARRAYSIZE(buffer))) {
                                    field["value"] = std::string(buffer);
                                    instanceChanged = true;
                                }
                            } else {
                                ImGui::TextDisabled("<%s>", fieldType.c_str());
                            }
                        }
                        if(instanceChanged) {
                            scriptComp->deserializeInstance(instanceData);
                            editorPage.documentManager.getActive()->markDirty();
                        }
                    }
                    ImGui::TreePop();
                    continue;
                }
                for(auto& property : classInfo->getAllProperties()) {
                    ImGui::Text(property->getName().c_str());
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(posX);
                    ImGui::SetNextItemWidth(width);
                    if(property->getTypeID() == Cube::getTypeID<float>()) {
                        float v = property->getValue(c).as<float>();
                        if(ImGui::DragFloat(("##" + property->getName()).c_str(), &v, 0.01, 0, 0, "%.3f")) {
                            editorPage.documentManager.getActive()->markDirty();
                            property->setValue(c, v);
                        }
                    } else if(property->getTypeID() == Cube::getTypeID<double>()) {
                        float v = (float)property->getValue(c).as<double>();
                        if(ImGui::DragFloat(("##" + property->getName()).c_str(), &v, 0.01, 0, 0, "%.3f")) {
                            editorPage.documentManager.getActive()->markDirty();
                            property->setValue(c, v);
                        }
                    } else if(property->getTypeID() == Cube::getTypeID<glm::vec2>()) {
                        glm::vec2 v = property->getValue(c).as<glm::vec2>();
                        float v2[2] = {v.x, v.y};
                        if(ImGui::DragFloat2(("##" + property->getName()).c_str(), v2, 0.01, 0, 0, "%.3f")) {
                            editorPage.documentManager.getActive()->markDirty();
                            property->setValue(c, glm::vec2(v2[0], v2[1]));
                        }
                    } else if(property->getTypeID() == Cube::getTypeID<glm::vec3>()) {
                        glm::vec3 v = property->getValue(c).as<glm::vec3>();
                        float v3[3] = {v.x, v.y, v.z};
                        if(ImGui::DragFloat3(("##" + property->getName()).c_str(), v3, 0.01, 0, 0, "%.3f")) {
                            editorPage.documentManager.getActive()->markDirty();
                            property->setValue(c, glm::vec3(v3[0], v3[1], v3[2]));
                        }
                    } else if(property->getTypeID() == Cube::getTypeID<glm::vec4>()) {
                        glm::vec4 v = property->getValue(c).as<glm::vec4>();
                        float v4[4] = {v.x, v.y, v.z, v.w};
                        if(ImGui::DragFloat4(("##" + property->getName()).c_str(), v4, 0.01, 0, 0, "%.3f")) {
                            editorPage.documentManager.getActive()->markDirty();
                            property->setValue(c, glm::vec4(v4[0], v4[1], v4[2], v4[3]));
                        }
                    } else if(property->getTypeID() == Cube::getTypeID<bool>()) {
                        bool v = property->getValue(c).as<bool>();
                        if(ImGui::Checkbox(("##" + property->getName()).c_str(), &v)) {
                            editorPage.documentManager.getActive()->markDirty();
                            property->setValue(c, v);
                        }
                    } else if(property->getTypeID() == Cube::getTypeID<int>()) {
                        int v = property->getValue(c).as<int>();
                        if(ImGui::DragInt(("##" + property->getName()).c_str(), &v, 1, 0, 0)) {
                            editorPage.documentManager.getActive()->markDirty();
                            property->setValue(c, v);
                        }
                    } else if(property->getTypeID() == Cube::getTypeID<std::string>()) {
                        char buffer[256] = {};
                        strcpy_s(buffer, property->getValue(c).as<std::string>().c_str());
                        if(ImGui::InputText(("##" + property->getName()).c_str(), buffer, IM_ARRAYSIZE(buffer))) {
                            editorPage.documentManager.getActive()->markDirty();
                            property->setValue(c, std::string(buffer));
                        }
                    } else if(property->getTypeID() == Cube::getTypeID<Cube::Color>()) {
                        Cube::Color color = property->getValue(c).as<Cube::Color>();
                        float colorV[4] = {color.r, color.g, color.b, color.a};
                        if(ImGui::ColorEdit4(("##" + property->getName()).c_str(), colorV, ImGuiColorEditFlags_DisplayHex)) {
                            editorPage.documentManager.getActive()->markDirty();
                            property->setValue(c, Cube::Color(colorV[0], colorV[1], colorV[2], colorV[3]));
                        }
                    } else if(property->getTypeID() == Cube::getTypeID<Cube::TextureRegion>()) {
                        glm::vec4 v = property->getValue(c).as<Cube::TextureRegion>().getUVCoord();
                        float v4[4] = {v.x, v.y, v.z, v.w};
                        if(ImGui::DragFloat4(("##" + property->getName()).c_str(), v4, 0.001f, 0, 1)) {
                            editorPage.documentManager.getActive()->markDirty();
                            Cube::TextureRegion tr;
                            tr.uvMin = {v4[0], v4[1]};
                            tr.uvMax = {v4[2], v4[3]};
                            property->setValue(c, tr);
                        }
                    } else if(property->getTypeID() == Cube::getTypeID<Cube::ResPtr<Cube::Sprite>>()) {
                        Cube::ResPtr<Cube::Sprite> res = property->getValue(c).as<Cube::ResPtr<Cube::Sprite>>();
                        if(res) {
                            ImVec2 size = Utils::keepAspectRatio(toImVec2(res->getSize()), 100.0f);
                            ImGui::Image(res->getTexture()->getId(), size, {res->getTexRegion().uvMin.x, res->getTexRegion().uvMax.y}, {res->getTexRegion().uvMax.x, res->getTexRegion().uvMin.y});
                        }else {
                            ImGui::Text("None");
                        }
                        if(ImGui::BeginDragDropTarget()) {
                            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Asset")) {
                                AssetNode* asset = *(AssetNode**)payload->Data;
                                if(asset->type == Cube::ResourceType::Texture) {
                                    property->setValue(c, Cube::ResPtr<Cube::Sprite>("spr:" + asset->identifier));
                                    editorPage.documentManager.getActive()->markDirty();
                                }
                            }
                            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("AssetSprite")){
                                std::string spriteIdentifier((char*)payload->Data, payload->DataSize);
                                property->setValue(c, Cube::ResPtr<Cube::Sprite>(spriteIdentifier));
                                editorPage.documentManager.getActive()->markDirty();
                            }
                            ImGui::EndDragDropTarget();
                        }
                    } else if (property->getTypeID() == Cube::getTypeID<std::unordered_map<std::string, Cube::ResPtr<Cube::AnimationClip>>>()) {
                        auto animClips = property->getValue(c).as<std::unordered_map<std::string, Cube::ResPtr<Cube::AnimationClip>>>();
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
                            resourcePickerDialog.open("Select Animation Clip", editorPage.getProject()->getAssetExplorer().getRootNode(), Cube::ResourceType::AnimationClip);
                        }
                        if(ImGui::BeginDragDropTarget()) {
                            if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Asset")) {
                                AssetNode* asset = *(AssetNode**)payload->Data;
                                if(asset->type == Cube::ResourceType::AnimationClip) {
                                    animClips[asset->identifier] = Cube::ResPtr<Cube::AnimationClip>(asset->identifier);
                                    property->setValue(c, animClips);
                                    editorPage.documentManager.getActive()->markDirty();
                                }
                            }
                            ImGui::EndDragDropTarget();
                        }
                        std::vector<std::string> pickedIdentifiers;
                        if(resourcePickerDialog.render(pickedIdentifiers, editorPage)) {
                            for(const auto& identifier : pickedIdentifiers) {
                                animClips[identifier] = Cube::ResPtr<Cube::AnimationClip>(identifier);
                            }
                            property->setValue(c, animClips);
                            editorPage.documentManager.getActive()->markDirty();
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
            EditorNodeAccess::removeComponent(*selectedNode, toDelete);
            editorPage.documentManager.getActive()->markDirty();
        }
        if(!scriptToDelete.empty()) {
            EditorNodeAccess::removeComponent(*selectedNode, scriptToDelete);
            editorPage.documentManager.getActive()->markDirty();
        }

        float w = ImGui::GetContentRegionAvail().x;
        static bool show = false;
        if(ImGui::Button("Add Component", {w, 0})) {
            ImGui::OpenPopup("addComponent");
        }
        if(ImGui::BeginPopup("addComponent")) {
            if(ImGui::MenuItem("SpriteRender")) {
                EditorNodeAccess::addComponent<Cube::SpriteRender>(*selectedNode);
            }
            if(ImGui::MenuItem("Camera2D")) {
                EditorNodeAccess::addComponent<Cube::Camera2D>(*selectedNode);
            }
            if(ImGui::MenuItem("Animation")) {
                EditorNodeAccess::addComponent<Cube::Animation>(*selectedNode);
            }
            if(ImGui::MenuItem("Script")) {
                scriptPickerDialog.open("Select Script", editorPage.getProject()->getAssetExplorer().getRootNode(), Cube::ResourceType::Script);
            }
            ImGui::EndPopup();
        }

        static std::string pendingScriptIdentifier;
        static char scriptClassName[128] = "MyScript";
        static bool newScriptPopupOpen = false;
        static std::string scriptAddError;

        std::string pickedScript;
        if(scriptPickerDialog.render(pickedScript, editorPage)) {
            pendingScriptIdentifier = pickedScript;
            scriptAddError.clear();
            newScriptPopupOpen = true;
        }

        if(newScriptPopupOpen) {
            ImGui::OpenPopup("New Script Component");
        }
        if(ImGui::BeginPopupModalSuper("New Script Component", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("script: %s", pendingScriptIdentifier.c_str());
            if(!scriptAddError.empty()) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.35f, 0.35f, 1.0f));
                ImGui::TextWrapped("%s", scriptAddError.c_str());
                ImGui::PopStyleColor();
            }
            ImGui::Text("Class name:");
            if(ImGui::IsWindowAppearing()) {
                ImGui::SetKeyboardFocusHere();
            }
            ImGui::InputText("##scriptClassName", scriptClassName, IM_ARRAYSIZE(scriptClassName));

            constexpr float buttonWidth = 100.0f;
            constexpr float spacing = 100.0f;
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - (buttonWidth * 2 + ImGui::GetStyle().FramePadding.x * 2 + spacing) / 2);
            ImGui::BeginGroup();
            if(ImGui::Button("OK", ImVec2(buttonWidth, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                // 先构造并校验实例，避免把无法运行(类名错误)的脚本组件加入节点
                auto scriptComp = std::make_unique<Cube::ScriptComponent>(selectedNode, pendingScriptIdentifier, std::string(scriptClassName));
                if(scriptComp->getInstance()) {
                    EditorNodeAccess::addComponent(*selectedNode, std::move(scriptComp));
                    editorPage.documentManager.getActive()->markDirty();
                    scriptAddError.clear();
                    newScriptPopupOpen = false;
                    ImGui::CloseCurrentPopup();
                } else {
                    scriptAddError = "Failed to create a script instance, check that the class name exists in the script.";
                }
            }
            ImGui::SameLine(0.0f, spacing);
            if(ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
                newScriptPopupOpen = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndGroup();

            ImGui::EndPopup();
        }
    }
    ImGui::End();
}

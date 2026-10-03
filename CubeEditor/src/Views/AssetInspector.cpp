#include "AssetInspector.h"

#include <algorithm>
#include <string>

#include "Cube/Core/Engine.h"
#include "imgui/imgui.h"

#include "../Project/AssetExplorer.h"
#include "../App/EditorApp.h"
#include "../App/EditorPage.h"
#include "../Project/Project.h"
#include "../Utils/EditorTextureCache.h"
#include "../Utils/ImGuiExternal.h"
#include "../Utils/misc.h"
#include "../Utils/FileDialog.h"
#include "AnimationEditor.h"

void AssetInspector::render(float deltaTime) {
    ImGui::Begin("Asset Inspector");
    AssetNode* node = editorPage.selectedAssetNode;
    AssetExplorer& assetExplorer = editorPage.getProject()->getAssetExplorer();
    if (node && !node->isGroup) {
        static std::string editingIdentifier;
        static nlohmann::json editingImporter;

        ImGui::Text("identifier: %s", node->identifier.c_str());
        auto importerRef = assetExplorer.getAssetImporter(node->identifier);
        if(!importerRef) {
            ImGui::Text("No importer");
            ImGui::End();
            return;
        }
        const nlohmann::json& importer = importerRef->get();
        if(editingIdentifier != node->identifier) {
            editingIdentifier = node->identifier;
            editingImporter = importer;
        }

        switch(node->type){
            case Cube::ResourceType::Texture:{
                static int gridRows = 1;
                static int gridCols = 1;

                ImGui::Text("path:");
                ImGui::Text("%s", editingImporter.value("path", "").c_str());
                ImGui::Separator();

                if(ImGui::Button("Load Sprites From Atlas")) {
                    // TODO: 设置过滤器
                    Cube::Path atlasPath = Utils::FileDialog::openFile("Select Atlas File");
                    if(!atlasPath.empty()) {
                        editingImporter["sprites"] = Utils::parseAtlasFile(atlasPath);
                    }
                }
                ImGui::SameLine();
                if(ImGui::Button("Grid Slice")) {
                    gridRows = 1;
                    gridCols = 1;
                    ImGui::OpenPopup("Grid Slice Texture");
                }

                if(ImGui::BeginPopupModalSuper("Grid Slice Texture", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui::Text("Split texture into row/column grid:");
                    ImGui::InputInt("Rows", &gridRows);
                    ImGui::InputInt("Columns", &gridCols);
                    if(gridRows < 1) gridRows = 1;
                    if(gridCols < 1) gridCols = 1;

                    ImGui::Separator();
                    if(ImGui::Button("Apply", ImVec2(120.0f, 0.0f))) {
                        nlohmann::json sprites = nlohmann::json::object();
                        for(int r = 0; r < gridRows; ++r) {
                            for(int c = 0; c < gridCols; ++c) {
                                const float u0 = static_cast<float>(c) / static_cast<float>(gridCols);
                                const float v0 = static_cast<float>(gridRows - r - 1) / static_cast<float>(gridRows);
                                const float u1 = static_cast<float>(c + 1) / static_cast<float>(gridCols);
                                const float v1 = static_cast<float>(gridRows - r) / static_cast<float>(gridRows);
                                const std::string name = "r" + std::to_string(r) + "_c" + std::to_string(c);
                                sprites[name] = {u0, v0, u1, v1};
                            }
                        }
                        editingImporter["sprites"] = std::move(sprites);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if(ImGui::Button("Cancel", ImVec2(120.0f, 0.0f))) {
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::EndPopup();
                }
                

                const std::string texturePath = editingImporter.value("path", "");
                if(!texturePath.empty()) {
                    Cube::Texture2D* previewTexture = EditorTextureCache::get().request(texturePath);

                    if(previewTexture) {
                        const float availWidth = ImGui::GetContentRegionAvail().x;
                        const float texW = static_cast<float>(previewTexture->getWidth());
                        const float texH = static_cast<float>(previewTexture->getHeight());
                        const float scale = texW > 0.0f ? std::min(1.0f, availWidth / texW) : 1.0f;
                        const ImVec2 previewSize = {texW * scale, texH * scale};

                        ImGui::Text("Preview:");
                        ImGui::Image(previewTexture->getId(), previewSize, {0, 1}, {1, 0});

                        if(editingImporter.contains("sprites") && editingImporter["sprites"].is_object()) {
                            const ImVec2 imageMin = ImGui::GetItemRectMin();
                            const ImVec2 imageMax = ImGui::GetItemRectMax();
                            const float imageW = imageMax.x - imageMin.x;
                            const float imageH = imageMax.y - imageMin.y;
                            ImDrawList* drawList = ImGui::GetWindowDrawList();

                            int idx = 0;
                            for(const auto& spriteEntry : editingImporter["sprites"].items()) {
                                const auto& v = spriteEntry.value();
                                if(!v.is_array() || v.size() < 4) {
                                    continue;
                                }

                                const float u0 = v[0].get<float>();
                                const float v0 = 1 - v[3].get<float>();
                                const float u1 = v[2].get<float>();
                                const float v1 = 1 - v[1].get<float>();

                                const ImVec2 p0 = {imageMin.x + u0 * imageW, imageMin.y + v0 * imageH};
                                const ImVec2 p1 = {imageMin.x + u1 * imageW, imageMin.y + v1 * imageH};

                                const ImU32 color = ImGui::GetColorU32(ImVec4(
                                    0.2f + ((idx * 37) % 100) / 140.0f,
                                    0.3f + ((idx * 53) % 100) / 160.0f,
                                    0.7f + ((idx * 29) % 100) / 350.0f,
                                    1.0f
                                ));
                                drawList->AddRect(p0, p1, color, 0.0f, 0, 1.5f);
                                ++idx;
                            }
                            drawList->AddRect(imageMin, imageMax, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), 0.0f, 0, 1.5f);
                        }
                    }
                }
                break;
            }
            case Cube::ResourceType::AnimationClip:{
                ImGui::Text("path:");
                ImGui::Text("%s", editingImporter.value("path", "").c_str());
                if(ImGui::Button("Edit in Animation Editor")) {
                    Cube::Engine::getApp()->getEventDispatcher().dispatch(AnimationEditor::TargetChangeEvent(Cube::Path(editingImporter.value("path", ""))));
                }
                break;
            } 
            case Cube::ResourceType::Font:{
                ImGui::Text("fontFilePath:");
                ImGui::Text("%s", editingImporter.value("fontFilePath", "").c_str());
                ImGui::Text("fontSize:");
                int fontSize = editingImporter.value("fontSize", 16);
                if(ImGui::InputInt("##fontSize", &fontSize)) {
                    if(fontSize < 1) {
                        fontSize = 1;
                    }
                    editingImporter["fontSize"] = fontSize;
                }
                break;
            }
            default: break;
        }
        if(ImGui::Button("Reimport")) {
            assetExplorer.reimportResource(node->identifier, editingImporter);
            if(auto updated = assetExplorer.getAssetImporter(node->identifier)) {
                editingImporter = updated->get();
            }
        }
        ImGui::Separator();
        // DEBUG
        ImGui::Text("importer:");
        ImGui::BeginChild("importerJson", ImVec2(0, 500), true);
        ImGui::Text("%s", editingImporter.dump(4).c_str());
        ImGui::EndChild();
    } else {
        ImGui::Text("No Asset Selected");
    }

    ImGui::End();
}
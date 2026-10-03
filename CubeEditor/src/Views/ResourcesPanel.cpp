#include "ResourcesPanel.h"

#include <memory>
#include <unordered_set>

#include "../App/EditorPage.h"
#include "../Project/Project.h"
#include "../Utils/EditorTextureCache.h"
#include "../Utils/ImGuiExternal.h"
#include "AnimationEditor.h"
#include "App/EditorApp.h"
#include "Cube/Core/Engine.h"
#include "Cube/Resource/ResourceType.h"
#include "imgui/imgui.h"

struct SelectedManager {
    std::unordered_set<AssetNode*> nodes;

    void singleSelect(AssetNode* node) {
        nodes.clear();
        nodes.insert(node);
    }

    void multiSelect(AssetNode* node) {
        nodes.insert(node);
    }

    bool isSelected(AssetNode* node) {
        return nodes.count(node);
    }

    void cancel() {
        nodes.clear();
    }

    AssetNode* getSingleNode() const {
        return nodes.empty() ? nullptr : *(nodes.begin());
    }

    const std::unordered_set<AssetNode*>& getNodes() const {
        return nodes;
    }
};

void ResourcesPanel::render(float deltaTime) {
    Project* project = editorPage.getProject();
    AssetExplorer& assetExplorer = project->getAssetExplorer();
    EditorTextureCache& textureCache = EditorTextureCache::get();
    Cube::Texture2D* back_png = textureCache.request("assets/icons/back.png");
    Cube::Texture2D* icon_mode_png = textureCache.request("assets/icons/icon_mode.png");
    Cube::Texture2D* list_mode_png = textureCache.request("assets/icons/list_mode.png");
    Cube::Texture2D* directory_png = textureCache.request("assets/icons/directory.png");
    Cube::Texture2D* file_png = textureCache.request("assets/icons/file.png");
    if(!back_png || !icon_mode_png || !list_mode_png || !directory_png || !file_png) {
        return;
    }

    static int showMode = 0; // 0: icon mode 1: list mode
    ImGui::Begin("Resources Panel");
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

    const std::string topText = assetExplorer.getCurrentPath();
    float topHeight = ImGui::CalcTextSize(topText.c_str()).y;

    ImGui::BeginChild("TopBar", ImVec2(ImGui::GetContentRegionAvail().x, topHeight));
    topHeight -= ImGui::GetStyle().FramePadding.x * 2;
    if(ImGui::ImageButton("back", back_png->getId(), ImVec2(topHeight, topHeight), {0, 1}, {1, 0})) {
        assetExplorer.back();
    }
    ImGui::SameLine();
    ImGui::Text(topText.c_str());
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - topHeight - ImGui::GetStyle().FramePadding.x * 2);
    if(showMode == 0) {
        if(ImGui::ImageButton("iconMode", icon_mode_png->getId(), ImVec2(topHeight, topHeight))) {
            showMode = 1;
        }
    }else if(showMode == 1) {
        if(ImGui::ImageButton("listMode", list_mode_png->getId(), ImVec2(topHeight, topHeight))) {
            showMode = 0;
        }
    }
    ImGui::EndChild();

    ImGui::BeginChild("Content", ImGui::GetContentRegionAvail());
    constexpr float imageSize = 128.0f;
    static SelectedManager selectedManager;
    static std::unordered_set<std::string> expandedTextureNodes;
    struct {
        AssetNode* src = nullptr;
        AssetNode* dst = nullptr;
    } move;
    if(showMode == 0){
        for(const auto& entry : assetExplorer.getCurrentNode()->children) {
            const bool isTexture = !entry->isGroup && entry->type == Cube::ResourceType::Texture;
            const nlohmann::json* textureImporter = nullptr;
            Cube::Texture2D* textureThumbnail = nullptr;
            bool textureHasSprites = false;

            ImGui::SameLine();
            if(ImGui::GetContentRegionAvail().x < imageSize) {
                ImGui::NewLine();
            }
            if(entry->isGroup){
                iconTextButton(directory_png, entry->name, selectedManager.isSelected(entry.get()), ImVec2(imageSize, imageSize));
                if(ImGui::BeginDragDropTarget()) {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("Asset")) {
                        move.src = *(AssetNode**)payload->Data;
                        move.dst = entry.get();
                    }
                    ImGui::EndDragDropTarget();
                }
            }else {
                switch(entry->type) {
                    case Cube::ResourceType::Texture:
                        {
                            if(auto imp = assetExplorer.getAssetImporter(entry->identifier)) {
                                textureImporter = &imp->get();
                            }
                            textureThumbnail = editorPage.thumbnailManager.request((*textureImporter)["path"].get<std::string>());
                            if(!textureThumbnail) textureThumbnail = file_png;
                            textureHasSprites = textureImporter->contains("sprites") && (*textureImporter)["sprites"].is_object() && !(*textureImporter)["sprites"].empty();
                            iconTextButton(textureThumbnail, entry->name, selectedManager.isSelected(entry.get()), ImVec2(imageSize, imageSize));

                            if(textureHasSprites) {
                                const bool expanded = expandedTextureNodes.count(entry->identifier);
                                const ImVec2 rectMin = ImGui::GetItemRectMin();
                                const ImVec2 rectMax = ImGui::GetItemRectMax();
                                const float cx = rectMax.x - 12.0f;
                                const float cy = rectMin.y + 12.0f;
                                const float s = 4.5f;
                                ImDrawList* drawList = ImGui::GetWindowDrawList();
                                ImU32 triColor = ImGui::GetColorU32(ImGuiCol_Text);
                                if(expanded) {
                                    drawList->AddTriangleFilled({cx - s, cy - s * 0.5f}, {cx + s, cy - s * 0.5f}, {cx, cy + s}, triColor);
                                } else {
                                    drawList->AddTriangleFilled({cx - s * 0.5f, cy - s}, {cx - s * 0.5f, cy + s}, {cx + s, cy}, triColor);
                                }
                            }
                        }
                        break;
                    case Cube::ResourceType::AnimationClip:
                        iconTextButton(file_png, entry->name, selectedManager.isSelected(entry.get()), ImVec2(imageSize, imageSize));
                        break;
                    default:
                        iconTextButton(file_png, entry->name, selectedManager.isSelected(entry.get()), ImVec2(imageSize, imageSize));
                        break;
                }
                ImGui::PushStyleColor(ImGuiCol_PopupBg, {0, 0, 0, 0});
                ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0);
                if(ImGui::BeginDragDropSource()) {
                    AssetNode* src = entry.get();
                    ImGui::SetDragDropPayload("Asset", &src, sizeof(src));
                    if(src->type == Cube::ResourceType::Texture) {
                        Cube::Texture2D* tex = nullptr;
                        if(auto imp = assetExplorer.getAssetImporter(src->identifier)) {
                            tex = editorPage.thumbnailManager.request(imp->get()["path"].get<std::string>());
                        }
                        ImGui::Image(tex ? tex->getId() : file_png->getId(), {64, 64}, {0, 1}, {1, 0});
                    }
                    ImGui::EndDragDropSource();
                }
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
            }

            const bool leftClicked = ImGui::IsItemClicked(ImGuiMouseButton_Left);
            const bool leftDoubleClicked = ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);
            if(entry->isGroup && leftDoubleClicked) {
                assetExplorer.enterNode(entry.get());
                selectedManager.cancel();
            }else{
                if(leftClicked) {
                    if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
                        selectedManager.multiSelect(entry.get());
                    }else {
                        selectedManager.singleSelect(entry.get());
                    }
                }
            }
            if(ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
                selectedManager.singleSelect(entry.get());
                ImGui::OpenPopup("NodeRightMenu");
            }

            if(isTexture && textureHasSprites && leftDoubleClicked) {
                if(expandedTextureNodes.count(entry->identifier)) {
                    expandedTextureNodes.erase(entry->identifier);
                } else {
                    expandedTextureNodes.insert(entry->identifier);
                }
            }

            if(isTexture && textureHasSprites && expandedTextureNodes.count(entry->identifier)) {
                const auto& sprites = (*textureImporter)["sprites"];
                const std::string texturePath = (*textureImporter)["path"].get<std::string>();
                Cube::Texture2D* spritePreviewTexture = textureCache.request(texturePath);
                if(!spritePreviewTexture) {
                    spritePreviewTexture = file_png;
                }

                const float spriteItemSize = imageSize * 0.8f;
                const float spriteVerticalPadding = (imageSize - spriteItemSize) * 0.5f;
                for(const auto& spriteEntry : sprites.items()) {
                    ImGui::SameLine();
                    if(ImGui::GetContentRegionAvail().x < spriteItemSize) {
                        ImGui::NewLine();
                    }

                    Cube::TextureRegion region = {{0.0f, 0.0f}, {1.0f, 1.0f}};
                    if(spriteEntry.value().is_array() && spriteEntry.value().size() >= 4) {
                        region = {
                            {spriteEntry.value()[0].get<float>(), spriteEntry.value()[1].get<float>()},
                            {spriteEntry.value()[2].get<float>(), spriteEntry.value()[3].get<float>()}
                        };
                    }

                    ImGui::BeginGroup();
                    ImGui::Dummy(ImVec2(0.0f, spriteVerticalPadding));
                    iconTextButton(spritePreviewTexture, spriteEntry.key(), false, ImVec2(spriteItemSize, spriteItemSize), region);
                    ImGui::PushStyleColor(ImGuiCol_PopupBg, {0, 0, 0, 0});
                    ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 0);
                    if(ImGui::BeginDragDropSource()) {
                        std::string spriteIdentifier = "spr:" + entry->identifier + "#" + spriteEntry.key();
                        ImGui::SetDragDropPayload("AssetSprite", spriteIdentifier.c_str(), spriteIdentifier.size());
                        ImGui::Image((spritePreviewTexture ? spritePreviewTexture : file_png)->getId(), Utils::keepAspectRatio(toImVec2(spritePreviewTexture->getSize() * (region.uvMax - region.uvMin)), 64), {region.uvMin.x, region.uvMax.y}, {region.uvMax.x, region.uvMin.y});
                        ImGui::EndDragDropSource();
                    }
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor();
                    ImGui::Dummy(ImVec2(0.0f, spriteVerticalPadding));
                    ImGui::EndGroup();
                }
            }
        }
    } else if(showMode == 1){
        // for(const auto& entry : proj->assetExplorer.getCurrentNode()->children) {
        //     if(entry->isGroup){
        //         iconTextButtonH(directory_png.get(), entry->name, selectedEntry == entry.get());
        //         if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        //             proj->assetExplorer.enterNode(entry.get());
        //         }
        //     }else {
        //         iconTextButtonH(file_png.get(), entry->name, selectedEntry == entry.get());
        //     }
        //     if(ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        //         selectedEntry = entry.get();
        //     }
        //     if(ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
        //         ImGui::OpenPopup("NodeRightButtonMenu");
        //     }
        // }
    }
    if(ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            selectedManager.cancel();
        }
        if(ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            selectedManager.cancel();
            ImGui::OpenPopup("BlankRightMenu");
        }
    }

    static std::vector<AssetNode*> cutNodes;
    if(ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) {
        // Ctrl+X - cut
        if(ImGui::IsKeyPressed(ImGuiKey_X)) {
            if(!cutNodes.empty()) cutNodes.clear();
            for(auto& n : selectedManager.getNodes()) {
                cutNodes.push_back(n);
            }
        }
        // Ctrl+V - paste
        if(ImGui::IsKeyPressed(ImGuiKey_V)) {
            for(auto& n : cutNodes) {
                assetExplorer.move(n, assetExplorer.getCurrentNode());
            }
            cutNodes.clear();
        }
    }

    // delay move
    if(move.src && move.dst) {
        assetExplorer.move(move.src, move.dst);
    }

    static char inputBuf[50] = {};
    static bool renamePopupOpen = false;

    if(renamePopupOpen) {
        ImGui::OpenPopup("Rename");
    }
    if(ImGui::BeginPopupModalSuper("Rename", &renamePopupOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Name:");
        if(ImGui::IsWindowAppearing()) {
            ImGui::SetKeyboardFocusHere();
        }
        ImGui::InputText("##rename", inputBuf, IM_ARRAYSIZE(inputBuf), ImGuiInputTextFlags_AutoSelectAll);

        constexpr float buttonWidth = 100.0f;
        constexpr float spacing = 100.0f;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - (buttonWidth * 2 + ImGui::GetStyle().FramePadding.x * 2 + spacing) / 2);
        ImGui::BeginGroup();
        if(ImGui::Button("OK", ImVec2(buttonWidth, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            if(selectedManager.getSingleNode()) {
                selectedManager.getSingleNode()->name = inputBuf;
            }
            renamePopupOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(0.0f, spacing);
        if(ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            renamePopupOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndGroup();

        ImGui::EndPopup();
    }

    if(ImGui::BeginPopup("NodeRightMenu")) {
        if(selectedManager.getSingleNode()->isGroup) {
            if(ImGui::MenuItem("Rename")) {
                strcpy_s(inputBuf, selectedManager.getSingleNode()->name.c_str());
                renamePopupOpen = true;
            }
        }
        if(selectedManager.getSingleNode()->type == Cube::ResourceType::AnimationClip){
            if(ImGui::MenuItem("Edit")){
                if(auto imp = assetExplorer.getAssetImporter(selectedManager.getSingleNode()->identifier)) {
                    const Cube::Path animPath(imp->get()["path"].get<std::string>());
                    Cube::Engine::getApp()->getEventDispatcher().dispatch(AnimationEditor::TargetChangeEvent(animPath));
                }
            }
        }
        if(ImGui::MenuItem("Delete")) {
            assetExplorer.removeNode(selectedManager.getSingleNode());
            selectedManager.cancel();
        }
        ImGui::EndPopup();
    }
    if(ImGui::BeginPopup("BlankRightMenu")) {
        if(ImGui::MenuItem("New Group")) {
            selectedManager.singleSelect(assetExplorer.createGroup("Group"));
            strcpy_s(inputBuf, selectedManager.getSingleNode()->name.c_str());
            renamePopupOpen = true;
        }
        ImGui::EndPopup();
    }

    editorPage.selectedAssetNode = selectedManager.getSingleNode();

    ImGui::EndChild();

    ImGui::PopStyleColor();

    ImGui::End();

    // ImGui::Begin("ResourcesPreview");
    //
    // ImGui::End();
}


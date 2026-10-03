#include "ResourcePickerDialog.h"

#include <algorithm>
#include <unordered_map>

#include "../Utils/EditorTextureCache.h"
#include "../Utils/ImGuiExternal.h"
#include "ThumbnailManager.h"
#include "Cube/Renderer/TextureRegion.h"
#include "../App/EditorPage.h"
#include "../Project/Project.h"

#include "imgui/imgui.h"

namespace {

struct DisplayItem {
    enum class Kind { Group, Resource, Sprite } kind = Kind::Resource;
    AssetNode* node = nullptr;
    std::string identifier;
    std::string label;
    Cube::Texture2D* icon = nullptr;
    Cube::TextureRegion uv = {{0.0f, 0.0f}, {1.0f, 1.0f}};
};

} // namespace

void ResourcePickerDialog::open(const std::string& popupTitle,
                                AssetNode* root,
                                Cube::ResourceType type) {
    title = popupTitle;
    rootNode = root;
    currentNode = rootNode;
    navStack.clear();
    allowedType = type;
    selectedIdentifiers.clear();
    selectedIdentifierSet.clear();
    selectionAnchorIndex = -1;
    expandedTextures.clear();
    isOpen = true;
    openRequested = true;
}

AssetNode* ResourcePickerDialog::getCurrentNode() const {
    if(currentNode) {
        return currentNode;
    }
    return rootNode;
}

bool ResourcePickerDialog::render(std::vector<std::string>& pickedIdentifiers, EditorPage& editorPage) {
    const std::unordered_map<std::string, nlohmann::json>& assetPathMap = editorPage.getProject()->getAssetExplorer().getAssetPathMap();

    if(!isOpen || !rootNode) {
        return false;
    }

    if(openRequested) {
        ImGui::OpenPopup(title.c_str());
        openRequested = false;
    }

    bool confirmed = false;
    ImGui::SetNextWindowSize(ImVec2(920.0f, 620.0f), ImGuiCond_Appearing);
    if(ImGui::BeginPopupModalSuper(title.c_str(), &isOpen, ImGuiWindowFlags_NoResize)) {
        AssetNode* node = getCurrentNode();
        if(!node) {
            ImGui::Text("No resource tree loaded.");
        } else {
            ImVec2 textSize = ImGui::CalcTextSize(currentNode->name.c_str());
            float textX = ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;
            if(!navStack.empty()) {
                float buttonHeight = ImGui::GetTextLineHeight() - ImGui::GetStyle().FramePadding.y * 2;
                ImVec2 buttonSize = ImVec2(buttonHeight, buttonHeight);
                if(ImGui::ImageButton("back", EditorTextureCache::get().request("assets/icons/back.png")->getId(), buttonSize, {0, 1}, {1, 0})) {
                    currentNode = navStack.back();
                    navStack.pop_back();
                    selectedIdentifiers.clear();
                    selectedIdentifierSet.clear();
                    selectionAnchorIndex = -1;
                }
                ImGui::SameLine();
            }
            
            ImGui::SetCursorPosX(textX);
            ImGui::Text("%s", currentNode->name.c_str());
            ImGui::Separator();

            EditorTextureCache& cache = EditorTextureCache::get();
            Cube::Texture2D* folderIcon = cache.request("assets/icons/directory.png");
            Cube::Texture2D* fileIcon = cache.request("assets/icons/file.png");

            std::vector<DisplayItem> items;
            items.reserve(node->children.size());

            for(const auto& child : node->children) {
                if(child->isGroup) {
                    items.push_back({DisplayItem::Kind::Group, child.get(), "", child->name, folderIcon});
                    continue;
                }

                const bool isNoFilter = allowedType == Cube::ResourceType::Unknown;
                const bool isSpriteFilter = allowedType == Cube::ResourceType::Sprite;
                const bool isTypeAllowed = isNoFilter || child->type == allowedType;
                const bool canShowTextureSprites = isSpriteFilter && child->type == Cube::ResourceType::Texture;

                if(isTypeAllowed || canShowTextureSprites) {
                    DisplayItem item;
                    item.kind = DisplayItem::Kind::Resource;
                    item.node = child.get();
                    item.label = child->name;
                    item.icon = fileIcon;
                    if(child->type == Cube::ResourceType::Texture) {
                        auto importerIt = assetPathMap.find(child->identifier);
                        if(importerIt != assetPathMap.end()) {
                            const std::string texPath = importerIt->second.value("path", "");
                            Cube::Texture2D* thumb = editorPage.thumbnailManager.request(texPath);
                            if(thumb) {
                                item.icon = thumb;
                            }
                        }
                    }
                    if(canShowTextureSprites) {
                        item.identifier = "spr:" + child->identifier;
                    } else {
                        item.identifier = child->identifier;
                    }
                    items.push_back(std::move(item));
                }

                if(canShowTextureSprites) {
                    auto importerIt = assetPathMap.find(child->identifier);
                    if(importerIt == assetPathMap.end()) {
                        continue;
                    }
                    const nlohmann::json& importer = importerIt->second;
                    if(importer.contains("sprites") && importer["sprites"].is_object() && expandedTextures.count(child->identifier)) {
                        const std::string texPath = importer.value("path", "");
                        Cube::Texture2D* tex = cache.request(texPath);
                        for(const auto& spriteEntry : importer["sprites"].items()) {
                            DisplayItem item;
                            item.kind = DisplayItem::Kind::Sprite;
                            item.node = child.get();
                            item.identifier = "spr:" + child->identifier + "#" + spriteEntry.key();
                            item.label = spriteEntry.key();
                            item.icon = tex ? tex : fileIcon;
                            if(spriteEntry.value().is_array() && spriteEntry.value().size() >= 4) {
                                item.uv = {
                                    {spriteEntry.value()[0].get<float>(), spriteEntry.value()[1].get<float>()},
                                    {spriteEntry.value()[2].get<float>(), spriteEntry.value()[3].get<float>()}
                                };
                            }
                            items.push_back(std::move(item));
                        }
                    }
                }
            }

            const float itemSize = 108.0f;
            float availWidth = ImGui::GetContentRegionAvail().x;
            int columns = static_cast<int>(availWidth / itemSize);
            if(columns < 1) columns = 1;

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
            ImGui::BeginChild("ResourcePickerGridArea", ImVec2(0.0f, -70.0f), false);
            bool clickedOnAnyItem = false;
            if(ImGui::BeginTable("ResourcePickerGrid", columns, ImGuiTableFlags_SizingFixedFit)) {
                for(size_t i = 0; i < items.size(); ++i) {
                    auto& item = items[i];
                    ImGui::TableNextColumn();
                    ImGui::PushID(static_cast<int>(i));

                    bool selected = !item.identifier.empty() && selectedIdentifierSet.count(item.identifier) > 0;
                    if(item.kind == DisplayItem::Kind::Group) {
                        selected = false;
                    }
                    iconTextButton(item.icon ? item.icon : fileIcon, item.label, selected, ImVec2(itemSize, itemSize), item.uv);

                    if(ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
                        clickedOnAnyItem = true;
                        if(item.kind == DisplayItem::Kind::Group) {
                            selectedIdentifiers.clear();
                            selectedIdentifierSet.clear();
                            selectionAnchorIndex = -1;
                        } else {
                            const ImGuiIO& io = ImGui::GetIO();
                            const bool ctrlPressed = io.KeyCtrl;
                            const bool shiftPressed = io.KeyShift;

                            if(shiftPressed && selectionAnchorIndex >= 0) {
                                const int begin = (std::min)(selectionAnchorIndex, static_cast<int>(i));
                                const int end = (std::max)(selectionAnchorIndex, static_cast<int>(i));

                                if(!ctrlPressed) {
                                    selectedIdentifiers.clear();
                                    selectedIdentifierSet.clear();
                                }

                                for(int idx = begin; idx <= end; ++idx) {
                                    const auto& rangeItem = items[static_cast<size_t>(idx)];
                                    if(rangeItem.kind == DisplayItem::Kind::Group || rangeItem.identifier.empty()) {
                                        continue;
                                    }
                                    if(selectedIdentifierSet.insert(rangeItem.identifier).second) {
                                        selectedIdentifiers.push_back(rangeItem.identifier);
                                    }
                                }
                            } else if(ctrlPressed) {
                                auto it = selectedIdentifierSet.find(item.identifier);
                                if(it != selectedIdentifierSet.end()) {
                                    selectedIdentifierSet.erase(it);
                                    selectedIdentifiers.erase(
                                        std::remove(selectedIdentifiers.begin(), selectedIdentifiers.end(), item.identifier),
                                        selectedIdentifiers.end());
                                } else {
                                    selectedIdentifierSet.insert(item.identifier);
                                    selectedIdentifiers.push_back(item.identifier);
                                    if(selectionAnchorIndex < 0) {
                                        selectionAnchorIndex = static_cast<int>(i);
                                    }
                                }
                            } else {
                                selectedIdentifiers.clear();
                                selectedIdentifierSet.clear();
                                selectedIdentifiers.push_back(item.identifier);
                                selectedIdentifierSet.insert(item.identifier);
                                selectionAnchorIndex = static_cast<int>(i);
                            }

                            if(selectionAnchorIndex < 0 && !selectedIdentifiers.empty()) {
                                selectionAnchorIndex = static_cast<int>(i);
                            }
                        }
                    }

                    if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
                        if(item.kind == DisplayItem::Kind::Group) {
                            navStack.push_back(currentNode);
                            currentNode = item.node;
                            selectedIdentifiers.clear();
                            selectedIdentifierSet.clear();
                            selectionAnchorIndex = -1;
                        } else if(item.kind == DisplayItem::Kind::Resource && allowedType == Cube::ResourceType::Sprite && item.node && item.node->type == Cube::ResourceType::Texture) {
                            if(expandedTextures.count(item.node->identifier)) {
                                expandedTextures.erase(item.node->identifier);
                            } else {
                                expandedTextures.insert(item.node->identifier);
                            }
                        }
                    }

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
            if(ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)
               && ImGui::IsMouseClicked(ImGuiMouseButton_Left)
               && !clickedOnAnyItem
               && !ImGui::IsAnyItemHovered()) {
                selectedIdentifiers.clear();
                selectedIdentifierSet.clear();
                selectionAnchorIndex = -1;
            }
            ImGui::EndChild();
            ImGui::PopStyleColor(1);
        }

        // Center buttons at the bottom
        float windowWidth = ImGui::GetWindowWidth();
        constexpr float buttonWidth = 120.0f;
        constexpr float spacing = 100.0f;
        constexpr float totalWidth = buttonWidth * 2 + spacing;
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 50.0f);
        ImGui::Separator();
        ImGui::SetCursorPosX((windowWidth - totalWidth) * 0.5f);
        if(ImGui::Button("Select", ImVec2(buttonWidth, 0))) {
            if(!selectedIdentifiers.empty()) {
                pickedIdentifiers = selectedIdentifiers;
                selectedIdentifiers.clear();
                selectedIdentifierSet.clear();
                selectionAnchorIndex = -1;
                isOpen = false;
                confirmed = true;
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine(0.0f, spacing);
        if(ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            isOpen = false;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    return confirmed;
}

bool ResourcePickerDialog::render(std::string& pickedIdentifier, EditorPage& editorPage) {
    std::vector<std::string> pickedIdentifiers;
    if(!render(pickedIdentifiers, editorPage) || pickedIdentifiers.empty()) {
        return false;
    }
    pickedIdentifier = pickedIdentifiers.front();
    return true;
}

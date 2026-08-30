#include "AnimationEditor.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "../App/EditorPage.h"
#include "../Project/AssetExplorer.h"
#include "../Project/Project.h"
#include "../Utils/EditorTextureCache.h"
#include "../Utils/ImGuiExternal.h"
#include "Cube/Core/Log.h"
#include "Cube/Renderer/TextureRegion.h"
#include "Cube/Core/Engine.h"
#include "json.hpp"

#include <imgui/imgui.h>

namespace {

bool resolveSpritePreview(const std::string& spriteIdentifier,
                        AssetExplorer& assetExplorer,
                        Cube::Texture2D*& texture,
                        Cube::TextureRegion& texRegion) {
    texture = nullptr;
    texRegion = {{0.0f, 0.0f}, {1.0f, 1.0f}};

    if(spriteIdentifier.rfind("spr:", 0) != 0) {
        return false;
    }

    const std::string resourcePart = spriteIdentifier.substr(4);
    const size_t hashPos = resourcePart.find('#');
    const std::string texIdentifier = hashPos == std::string::npos ? resourcePart : resourcePart.substr(0, hashPos);

    nlohmann::json importer;
    try {
        importer = assetExplorer.getAssetImporter(texIdentifier);
    } catch(const std::out_of_range&) {
        return false;
    }

    const std::string texPath = importer.value("path", "");
    if(texPath.empty()) {
        return false;
    }

    texture = EditorTextureCache::get().request(texPath);
    if(!texture) {
        return false;
    }

    if(hashPos != std::string::npos) {
        const std::string spriteName = resourcePart.substr(hashPos + 1);
        if(importer.contains("sprites") && importer["sprites"].is_object() && importer["sprites"].contains(spriteName)) {
            const auto& uv = importer["sprites"][spriteName];
            if(uv.is_array() && uv.size() >= 4) {
                texRegion = {
                    {uv[0].get<float>(), uv[1].get<float>()},
                    {uv[2].get<float>(), uv[3].get<float>()}
                };
            }
        }
    }

    return true;
}

std::string getFrameDisplayName(const std::string& spriteIdentifier) {
    std::string resourcePart = spriteIdentifier;
    if(resourcePart.rfind("spr:", 0) == 0) {
        resourcePart = resourcePart.substr(4);
    }

    const size_t hashPos = resourcePart.find('#');
    if(hashPos != std::string::npos && hashPos + 1 < resourcePart.size()) {
        return resourcePart.substr(hashPos + 1);
    }

    const std::string texIdentifier = hashPos == std::string::npos ? resourcePart : resourcePart.substr(0, hashPos);
    size_t nameStart = texIdentifier.find_last_of("/\\");
    if(nameStart == std::string::npos) {
        nameStart = texIdentifier.find(':');
        if(nameStart == std::string::npos) {
            nameStart = 0;
        } else {
            ++nameStart;
        }
    } else {
        ++nameStart;
    }

    std::string textureName = texIdentifier.substr(nameStart);
    const size_t extPos = textureName.find_last_of('.');
    if(extPos != std::string::npos) {
        textureName = textureName.substr(0, extPos);
    }
    return textureName.empty() ? texIdentifier : textureName;
}

} // namespace

bool AnimationEditor::createNewAnimationClip(const std::string& fileName) {
    Project* project = editorPage.getProject();
    if(!project) {
        // TODO: Show user-level error prompt in unified notification system.
        CB_EDITOR_ERROR("AnimationEditor: No active project when creating animation clip");
        return false;
    }

    const Cube::Path& assetsDir = project->getConfig().assetsDirectory;

    std::string baseName = fileName;
    if(baseName.empty()) {
        baseName = "NewAnimation";
    }
    if(baseName.size() > 5 && baseName.substr(baseName.size() - 5) == ".anim") {
        baseName = baseName.substr(0, baseName.size() - 5);
    }

    Cube::Path animPath = assetsDir / (baseName + ".anim");
    int index = 1;
    while(std::filesystem::exists(animPath.string())) {
        animPath = assetsDir / (baseName + "_" + std::to_string(index) + ".anim");
        ++index;
    }

    nlohmann::json animData;
    animData["name"] = animPath.stem();
    animData["looping"] = true;
    animData["speed"] = 1.0f;
    animData["duration"] = 0.0f;
    animData["frames"] = nlohmann::json::array();

    std::ofstream file(animPath.string());
    if(!file.is_open()) {
        // TODO: Show user-level error prompt in unified notification system.
        CB_EDITOR_ERROR("AnimationEditor: Failed to create animation file {}", animPath);
        return false;
    }
    file << animData.dump(4);
    file.close();

    Cube::Path relPath = animPath.lexicallyRelative(assetsDir);

    nlohmann::json importConfig;
    importConfig["path"] = animPath.string();
    project->getAssetExplorer().createResource("anim:" + relPath.string(), importConfig);

    TargetChangeEvent e(animPath);
    Cube::Engine::getApp()->getEventDispatcher().dispatch(e);
    return true;
}

bool AnimationEditor::loadTargetAnim() {
    frames.clear();
    selectedFrameIndex = -1;
    isPreviewPlaying = false;
    previewTime = 0.0f;
    name.clear();
    looping = false;
    speed = 1.0f;
    duration = 0.0f;

    if(target.empty()) {
        return false;
    }

    std::ifstream file(target.string());
    if(!file.is_open()) {
        // TODO: Show user-level error prompt in unified notification system.
        CB_EDITOR_ERROR("AnimationEditor: Failed to open animation file {}", target);
        return false;
    }

    nlohmann::json animData;
    try {
        file >> animData;
    } catch(const nlohmann::json::exception& e) {
        // TODO: Show user-level error prompt in unified notification system.
        CB_EDITOR_ERROR("AnimationEditor: Invalid json file {}: {}", target, e.what());
        return false;
    }

    name = animData.value("name", "");
    looping = animData.value("looping", false);
    speed = animData.value("speed", 1.0f);
    duration = 0.0f;

    if(animData.contains("frames") && animData["frames"].is_array()) {
        for(const auto& frameJson : animData["frames"]) {
            FrameViewData f;
            f.frame = frameJson.value("frame", "");
            f.duration = frameJson.value("duration", 0.0f);
            if(f.duration < 0.0f) {
                f.duration = 0.0f;
            }
            duration += f.duration;
            frames.push_back(std::move(f));
        }
    }

    return true;
}

bool AnimationEditor::saveTargetAnim() {
    if(target.empty()) {
        CB_EDITOR_ERROR("AnimationEditor: No target file to save");
        return false;
    }

    nlohmann::json animData;
    animData["name"] = name;
    animData["looping"] = looping;
    animData["speed"] = speed;

    float totalDuration = 0.0f;
    animData["frames"] = nlohmann::json::array();
    for(const auto& frame : frames) {
        nlohmann::json frameJson;
        frameJson["frame"] = frame.frame;
        frameJson["duration"] = std::max(0.0f, frame.duration);
        totalDuration += frameJson["duration"].get<float>();
        animData["frames"].push_back(std::move(frameJson));
    }
    animData["duration"] = totalDuration;

    std::ofstream file(target.string());
    if(!file.is_open()) {
        CB_EDITOR_ERROR("AnimationEditor: Failed to open animation file for save {}", target);
        return false;
    }

    file << animData.dump(4);
    file.close();
    duration = totalDuration;
    return true;
}

void AnimationEditor::render(float deltaTime) {
    ImGui::Begin("Animation Editor");

    static char newAnimName[128] = "NewAnimation";

    if(ImGui::Button("New AnimationClip")) {
        ImGui::OpenPopup("New AnimationClip");
    }
    ImGui::SameLine();
    if(ImGui::Button("Save")) {
        saveTargetAnim();
    }

    if(ImGui::BeginPopupModal("New AnimationClip", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Input animation file name:");
        ImGui::InputText("##NewAnimName", newAnimName, IM_ARRAYSIZE(newAnimName));

        if(ImGui::Button("Create", ImVec2(120.0f, 0.0f))) {
            createNewAnimationClip(newAnimName);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if(ImGui::Button("Cancel", ImVec2(120.0f, 0.0f))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Text("Target:");
    ImGui::SameLine();
    ImGui::Text("%s", target.empty() ? "<None>" : target.c_str());

    if(target.empty()) {
        ImGui::End();
        return;
    }

    float autoDuration = 0.0f;
    for(const auto& f : frames) {
        autoDuration += f.duration;
    }
    duration = autoDuration;

    if(isPreviewPlaying) {
        if(frames.empty() || duration <= 0.0f || speed <= 0.0f) {
            isPreviewPlaying = false;
            previewTime = 0.0f;
        } else {
            previewTime += deltaTime * speed;
            if(looping) {
                while(previewTime >= duration) {
                    previewTime -= duration;
                }
            } else if(previewTime >= duration) {
                previewTime = duration;
                isPreviewPlaying = false;
            }
        }
    }

    if(!isPreviewPlaying && previewTime > duration) {
        previewTime = duration;
    }

    AssetExplorer& assetExplorer = editorPage.getProject()->getAssetExplorer();

    ImGui::Separator();

    if(ImGui::BeginTable("Anim", 2, ImGuiTableFlags_Resizable)){
        ImGui::TableSetupColumn("Properties", ImGuiTableColumnFlags_WidthFixed, 250.0f);
        ImGui::TableSetupColumn("Frames", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableNextColumn();

        ImGui::BeginChild("AnimationPropertiesPane", ImVec2(0, 0), false);
        if(ImGui::BeginTable("AnimProperty", 2, ImGuiTableFlags_SizingFixedFit)){
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Name");
            ImGui::TableSetColumnIndex(1); ImGui::Text("%s", name.c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Looping");
            ImGui::TableSetColumnIndex(1); ImGui::Checkbox("##looping", &looping);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Speed");
            ImGui::TableSetColumnIndex(1);
            if(ImGui::DragFloat("##speed", &speed, 0.01f, 0.01f, 10.0f, "%.3f")) {
                if(speed < 0.0f) speed = 0.0f;
            }

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Duration (auto)");
            ImGui::TableSetColumnIndex(1); ImGui::Text("%.3f", duration);

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::Text("Frames");
            ImGui::TableSetColumnIndex(1); ImGui::Text("%d", static_cast<int>(frames.size()));

            ImGui::EndTable();
        }
        ImGui::Separator();
        ImGui::Text("Preview");
        if(isPreviewPlaying) {
            if(ImGui::Button("Stop")) {
                isPreviewPlaying = false;
            }
        } else {
            if(ImGui::Button("Play")) {
                if(!frames.empty()) {
                    if(duration <= 0.0f || previewTime >= duration) {
                        previewTime = 0.0f;
                    }
                    isPreviewPlaying = true;
                }
            }
        }
        ImGui::SameLine();
        if(ImGui::Button("Reset")) {
            isPreviewPlaying = false;
            previewTime = 0.0f;
        }

        int previewFrameIndex = -1;
        if(!frames.empty()) {
            if(duration <= 0.0f) {
                previewFrameIndex = 0;
            } else {
                float sampledTime = previewTime;
                if(sampledTime >= duration) {
                    sampledTime = duration - 0.0001f;
                }
                if(sampledTime < 0.0f) {
                    sampledTime = 0.0f;
                }

                float cursor = 0.0f;
                for(size_t i = 0; i < frames.size(); ++i) {
                    const float frameDuration = std::max(0.0f, frames[i].duration);
                    cursor += frameDuration;
                    if(sampledTime < cursor || i == frames.size() - 1) {
                        previewFrameIndex = static_cast<int>(i);
                        break;
                    }
                }
            }
        }

        ImGui::Text("time: %.3f / %.3f", previewTime, duration);
        if(previewFrameIndex >= 0 && previewFrameIndex < static_cast<int>(frames.size())) {
            const auto& previewFrame = frames[previewFrameIndex];
            Cube::Texture2D* previewTexture = nullptr;
            Cube::TextureRegion region = {{0.0f, 0.0f}, {1.0f, 1.0f}};
            if(resolveSpritePreview(previewFrame.frame, assetExplorer, previewTexture, region) && previewTexture) {
                const float regionW = static_cast<float>(previewTexture->getWidth()) * (region.uvMax.x - region.uvMin.x);
                const float regionH = static_cast<float>(previewTexture->getHeight()) * (region.uvMax.y - region.uvMin.y);
                const float maxSize = 120.0f;
                float scale = 1.0f;
                if(regionW > 0.0f && regionH > 0.0f) {
                    scale = std::min(maxSize / regionW, maxSize / regionH);
                }
                const ImVec2 drawSize = {regionW * scale, regionH * scale};
                ImGui::Image(previewTexture->getId(), drawSize, {region.uvMin.x, region.uvMax.y}, {region.uvMax.x, region.uvMin.y});
            } else {
                ImGui::TextDisabled("Preview unavailable");
            }
            ImGui::TextWrapped("frame: %s", getFrameDisplayName(previewFrame.frame).c_str());
        } else {
            ImGui::TextDisabled("No preview frame");
        }
        
        ImGui::EndChild();

        ImGui::TableNextColumn();

        ImGui::BeginChild("AnimationFramesPane", ImVec2(0, 0), false);
        ImGui::BeginGroup();
        ImGui::Text("Frames");
        ImGui::SameLine();
        if(ImGui::Button("Add Frame")) {
            framePickerDialog.open("Select Frame Resource",
                                assetExplorer.getRootNode(),
                                Cube::ResourceType::Sprite);
        }
        std::vector<std::string> pickedIdentifiers;
        if(framePickerDialog.render(pickedIdentifiers, editorPage)) {
            for(const auto& identifier : pickedIdentifiers) {
                FrameViewData f;
                f.frame = identifier;
                f.duration = 0.1f;
                frames.push_back(std::move(f));
            }
        }
        ImGui::SameLine();

        ImGui::EndGroup();
        ImGui::Separator();
        ImGui::BeginGroup();
        if(selectedFrameIndex >= 0 && selectedFrameIndex < static_cast<int>(frames.size())) {
            FrameViewData& selectedFrame = frames[selectedFrameIndex];
            if(ImGui::BeginTable("selected", 2, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInner)){
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0); ImGui::Text("index: %d", selectedFrameIndex);
                ImGui::TableSetColumnIndex(1); 
                ImGui::Text("duration:");
                ImGui::SameLine();
                if(ImGui::InputFloat("##duration", &selectedFrame.duration, 0.01f, 0.1f, "%.3f")) {
                    if(selectedFrame.duration < 0.0f) {
                        selectedFrame.duration = 0.0f;
                    }
                }
                ImGui::EndTable();
            }
            const bool canMoveLeft = selectedFrameIndex > 0;
            const bool canMoveRight = selectedFrameIndex >= 0 && selectedFrameIndex < static_cast<int>(frames.size()) - 1;

            if(!canMoveLeft) {
                ImGui::BeginDisabled();
            }
            if(ImGui::Button("Move Left")) {
                std::swap(frames[selectedFrameIndex], frames[selectedFrameIndex - 1]);
                --selectedFrameIndex;
            }
            if(!canMoveLeft) {
                ImGui::EndDisabled();
            }

            ImGui::SameLine();

            if(!canMoveRight) {
                ImGui::BeginDisabled();
            }
            if(ImGui::Button("Move Right")) {
                std::swap(frames[selectedFrameIndex], frames[selectedFrameIndex + 1]);
                ++selectedFrameIndex;
            }
            if(!canMoveRight) {
                ImGui::EndDisabled();
            }

            ImGui::SameLine();
            if(ImGui::Button("Delete")) {
                frames.erase(frames.begin() + selectedFrameIndex);
                if(frames.empty()) {
                    selectedFrameIndex = -1;
                } else if(selectedFrameIndex >= static_cast<int>(frames.size())) {
                    selectedFrameIndex = static_cast<int>(frames.size()) - 1;
                }
            }
        } else {
            ImGui::TextDisabled("No frame selected");
        }
        ImGui::EndGroup();
        ImGui::Separator();

        ImGui::BeginChild("AnimationFrames", ImVec2(0, 0), true);

        float itemWidth = 140.0f;
        float itemHeight = 140.0f;
        float padding = ImGui::GetStyle().ItemSpacing.x;
        float availWidth = ImGui::GetContentRegionAvail().x;
        float x = 0.0f;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        for(size_t i = 0; i < frames.size(); ++i) {
            const auto& frame = frames[i];
            ImGui::PushID(static_cast<int>(i));
            const bool selected = (selectedFrameIndex == static_cast<int>(i));

            Cube::Texture2D* previewTexture = nullptr;
            Cube::TextureRegion region = {{0.0f, 0.0f}, {1.0f, 1.0f}};
            const std::string displayName = getFrameDisplayName(frame.frame);
            if(resolveSpritePreview(frame.frame, assetExplorer, previewTexture, region) && previewTexture) {
                if(iconTextButton(previewTexture, displayName, selected, ImVec2(itemWidth, itemHeight), region)) {
                    selectedFrameIndex = static_cast<int>(i);
                }
            } else {
                if(ImGui::Button(displayName.c_str(), ImVec2(itemWidth, itemHeight))) {
                    selectedFrameIndex = static_cast<int>(i);
                }
            }

            x += itemWidth + padding;
            // If next item would exceed available width, move to next line
            if(i + 1 < frames.size() && x + itemWidth > availWidth) {
                x = 0.0f;
                ImGui::NewLine();
            } else {
                ImGui::SameLine();
            }

            ImGui::PopID();
        }
        ImGui::PopStyleColor();

        if(ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered()) {
            selectedFrameIndex = -1;
        }

        ImGui::EndChild();
        ImGui::EndChild();

        ImGui::EndTable();
    }
    ImGui::End();
}

bool AnimationEditor::onTargetChange(const Cube::Event& e) {
    const TargetChangeEvent& event = static_cast<const TargetChangeEvent&>(e);
    target = event.targetFilePath;
    loadTargetAnim();
    ImGui::SetWindowFocus("Animation Editor");
    return true;
}
#pragma once

#include <imgui/imgui.h>

#include <functional>
#include <string>

#include "Cube/Renderer/Color.h"
#include "Cube/Renderer/Texture.h"
#include "Cube/Renderer/TextureRegion.h"

void addDashLine(ImDrawList* drawList, const ImVec2& start, const ImVec2& end, const ImU32& color, float thickness = 1.0f, float segmentLen = 10.0f, float intervalLen = 10.0f);

class ModalPopup {
public:
    ModalPopup(const std::string& title, const std::function<void()>& content, const std::function<void()>& confirm, const std::function<void()>& clear);
    ~ModalPopup() = default;

    void render();
    void open() { isOpen = true; }
    void close() { isOpen = false; }
    bool getIsOpen() const { return isOpen; }

private:
    std::string title;
    bool isOpen = false;
    std::function<void()> content;
    std::function<void()> confirm;
    std::function<void()> clear;
    ImVec4 borderColor;
    ImVec4 originalBorderColor;
    ImVec4 highLightColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
};

// bool IconTextButton(ImTextureID tex_id, const char* label, const ImVec2& icon_size, const ImVec2& uv_min = {0, 0}, const ImVec2& uv_max = {1, 1}, ImGuiButtonFlags flags = ImGuiButtonFlags_None);

// bool IconTextButtonLeft(const char* label, ImTextureID tex_id, const ImVec2& uv_min = {0, 0}, const ImVec2& uv_max = {1, 1}, const ImVec2& button_size = {0, 0}, const ImVec2& icon_size = ImVec2(ImGui::CalcTextSize("Text").y, ImGui::CalcTextSize("Text").y));

bool iconTextButton(const Cube::Texture2D* icon, std::string_view label, bool isSelected = false, const ImVec2& size = {0, 0}, const Cube::TextureRegion& texUV = {{0, 0}, {1, 1}});
bool iconTextButtonH(const Cube::Texture2D* icon, std::string_view label, bool isSelected = false, const Cube::TextureRegion& texUV = {{0, 0}, {1, 1}});

bool editableLabel(const char* id, std::string& text, bool triggerEdit = false);

inline ImVec4 toImColor(const Cube::Color& color) {
    return {color.r, color.g, color.b, color.a};
}

inline ImVec2 toImVec2(const glm::vec2& vec) {
    return ImVec2(vec.x, vec.y);
}

inline glm::vec2 toGlmVec2(const ImVec2& vec) {
    return glm::vec2(vec.x, vec.y);
}

namespace Utils {

    constexpr ImGuiTreeNodeFlags TREENODE_FLAGS = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    // 保持长宽比的缩放
    inline ImVec2 keepAspectRatio(const ImVec2& size, float maxDimension) {
        float scale = maxDimension / std::max(size.x, size.y);
        return ImVec2(size.x * scale, size.y * scale);
    }
}

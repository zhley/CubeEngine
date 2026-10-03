#pragma once

#include <imgui/imgui.h>

#include <string>
#include <algorithm>

#include "Cube/Renderer/Color.h"
#include "Cube/Renderer/Texture.h"
#include "Cube/Renderer/TextureRegion.h"

void addDashLine(ImDrawList* drawList, const ImVec2& start, const ImVec2& end, const ImU32& color, float thickness = 1.0f, float segmentLen = 10.0f, float intervalLen = 10.0f);

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

    inline ImVec2 keepAspectRatio(const ImVec2& size, float maxDimension) {
        float scale = maxDimension / (std::max)(size.x, size.y);
        return ImVec2(size.x * scale, size.y * scale);
    }
}

namespace ImGui {

    // Enhanced modal popup window with feedback.
    // Compared to BeginPopupModal, when the user clicks/hover on a window outside the modal window, 
    // it will give feedback such as border highlighting/window flashing/shaking/system beep.
    struct ModalSuperStyle {
        bool         feedbackOnClickOutside = true;
        bool         feedbackOnHoverOutside = false; 
        bool         highlightBorder        = true; 
        bool         flashWindow            = true;
        bool         shakeWindow            = true;
        bool         playSound              = true;
        unsigned int beepType               = 0x00000040u; // MB_ICONASTERISK
        float        cooldown               = 0.30f;
        float        flashDuration          = 0.45f;
        float        shakeDuration          = 0.35f;
        float        shakeAmplitude         = 6.0f;
        ImVec4       highlightColor         = ImVec4(1.00f, 0.35f, 0.30f, 1.00f);
    };

    ModalSuperStyle& GetModalSuperStyle();

    bool BeginPopupModalSuper(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);

} // namespace ImGui

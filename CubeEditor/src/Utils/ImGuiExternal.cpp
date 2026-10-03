#include "ImGuiExternal.h"

#include "Cube/Core/Log.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

#include <cmath>
#include <unordered_map>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

void addDashLine(ImDrawList* drawList, const ImVec2& start, const ImVec2& end, const ImU32& color, float thickness, float segmentLen, float intervalLen) {
    ImVec2 delta = end - start;
    float len = std::sqrt(delta.x * delta.x + delta.y * delta.y);
    if(len < 1e-5f) {
        CB_EDITOR_ERROR("addDashLine: divided by zero");
        return;
    }
    ImVec2 unit = delta / len;
    float step = segmentLen + intervalLen;
    int count = (int)(len / step);
    for(int i = 0; i < count; ++i) {
        ImVec2 pos = start + unit * step * i;
        drawList->AddLine(pos, pos + unit * segmentLen, color, thickness);
    }
    if(step * count + segmentLen > len) {
        drawList->AddLine(start + unit * step * count, end, color, thickness);
    } else {
        drawList->AddLine(start + unit * step * count, start + unit * (step * count + segmentLen), color, thickness);
    }
}

bool iconTextButton(const Cube::Texture2D* icon, std::string_view label, bool isSelected, const ImVec2& size, const Cube::TextureRegion& texUV) {
    float rounding = ImGui::GetStyle().FrameRounding;
    float padding = ImGui::GetStyle().FramePadding.y;
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if(window->SkipItems) return false;

    ImVec2 iconSize = ImVec2(icon->getWidth(), icon->getHeight()) * toImVec2(texUV.uvMax - texUV.uvMin);
    const float sampledAspectRatio = iconSize.x > 0.0f ? (iconSize.y / iconSize.x) : 1.0f;
    ImVec2 textSize = ImGui::CalcTextSize(label.data());
    ImVec2 itemSize = ImVec2(ImMax(iconSize.x, textSize.x) + padding * 2, iconSize.y + textSize.y + padding * 3);
    if(size.x > 0) {
        itemSize.x = size.x;
        if(iconSize.x > size.x - padding * 2) {
            iconSize.x = size.x - padding * 2;
            iconSize.y = iconSize.x * sampledAspectRatio;
        }
    }
    if(size.y > 0) {
        itemSize.y = size.y;
        if(iconSize.y > size.y - textSize.y - padding * 3) {
            iconSize.y = size.y - textSize.y - padding * 3;
            iconSize.x = iconSize.y / sampledAspectRatio;
        }
    }

    static const float w = ImGui::CalcTextSize("...").x;
    float charWidth = ImGui::CalcTextSize("0").x;
    float maxTextWidth = ImMax(size.x, iconSize.x) - ImGui::GetStyle().FramePadding.y * 2;
    bool isTextHidden = false;
    std::string text = label.data();
    if(textSize.x > maxTextWidth) {
        int charCount = static_cast<int>((maxTextWidth - w) / charWidth);
        text = text.substr(0, charCount);
        text.append("...");
        textSize = ImGui::CalcTextSize(text.c_str());
        isTextHidden = true;
    }

    ImGui::InvisibleButton(label.data(), itemSize);

    bool isHovered = ImGui::IsItemHovered();
    bool isActive = ImGui::IsItemActive();
    bool isClicked = ImGui::IsItemClicked();

    ImU32 bgColor = ImGui::GetColorU32(isActive ? ImGuiCol_ButtonActive : isHovered ? ImGuiCol_ButtonHovered : isSelected ? ImGuiCol_TextSelectedBg : ImGuiCol_Button);
    
    ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), bgColor, rounding);

    if(isTextHidden && ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        ImGui::BeginTooltip();
        ImGui::Text(label.data());
        ImGui::EndTooltip();
    }

    ImVec2 iconPos = ImVec2(ImGui::GetItemRectMin().x + (itemSize.x - iconSize.x) * 0.5f, ImGui::GetItemRectMin().y + padding);

    ImGui::GetWindowDrawList()->AddImage(icon->getId(), iconPos, ImVec2(iconPos.x + iconSize.x, iconPos.y + iconSize.y), {texUV.uvMin.x, texUV.uvMax.y}, {texUV.uvMax.x, texUV.uvMin.y});

    ImVec2 textPos = ImVec2(ImGui::GetItemRectMin().x + (itemSize.x - textSize.x) * 0.5f, ImGui::GetItemRectMin().y + itemSize.y - padding - textSize.y);

    ImGui::GetWindowDrawList()->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), text.c_str());

    return isClicked;
}

bool iconTextButtonH(const Cube::Texture2D* icon, std::string_view label, bool isSelected, const Cube::TextureRegion& texUV) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if(window->SkipItems)
        return false;

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 padding = style.FramePadding;
    const float rounding = style.FrameRounding;

    ImVec2 textSize = ImGui::CalcTextSize(label.data());
    ImVec2 iconSize = ImVec2(icon->getWidth(), icon->getHeight()) * toImVec2(texUV.uvMax - texUV.uvMin);
    const float sampledAspectRatio = iconSize.x > 0.0f ? (iconSize.y / iconSize.x) : 1.0f;
    if(iconSize.x > textSize.y || iconSize.y > textSize.y) {
        if(sampledAspectRatio < 1.0f) {
            iconSize.x = textSize.y;
            iconSize.y = iconSize.x * sampledAspectRatio;
        } else {
            iconSize.y = textSize.y;
            iconSize.x = iconSize.y / sampledAspectRatio;
        }
    }
    float iconTextSpacing = 8.0f;
    ImVec2 buttonSize = {ImGui::GetContentRegionAvail().x, textSize.y + padding.y * 2};

    ImGui::InvisibleButton(label.data(), buttonSize);

    bool isHovered = ImGui::IsItemHovered();
    bool isActive = ImGui::IsItemActive();
    bool isClicked = ImGui::IsItemClicked();

    ImU32 bgColor = ImGui::GetColorU32(isActive ? ImGuiCol_ButtonActive : isHovered ? ImGuiCol_ButtonHovered : isSelected ? ImGuiCol_TextSelectedBg : ImGuiCol_Button);

    ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), bgColor, rounding);

    if(isHovered || isActive) {
        ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), ImGui::GetColorU32(ImGuiCol_Border), rounding, 0, 1.5f);
    }

    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 iconMin = pos + ImVec2((buttonSize.y - iconSize.x) * 0.5f, (buttonSize.y - iconSize.y) * 0.5f);
    window->DrawList->AddImage(icon->getId(), iconMin, iconMin + iconSize, {texUV.uvMin.x, texUV.uvMax.y}, {texUV.uvMax.x, texUV.uvMin.y});

    float textY = pos.y + (buttonSize.y - textSize.y) * 0.5f;
    ImVec2 textPos(pos.x + buttonSize.y + iconTextSpacing, textY);
    window->DrawList->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), label.data());

    return isClicked;
}

bool editableLabel(const char* id, std::string& text, bool triggerEdit) {
    // 用 id 作为 key 存储每个 label 的独立状态
    struct EditState {
        bool  isEditing = false;
        char  buf[256]  = {};
        bool  needFocus = false;
    };

    static std::unordered_map<std::string, EditState> s_states;

    EditState& state = s_states[id];
    bool committed = false;

    if (!state.isEditing) {
        // ---- Label 模式 ----
        ImGui::TextUnformatted(text.c_str());

        // 触发进入编辑模式：外部 triggerEdit 信号
        if (triggerEdit)
        {
            state.isEditing = true;
            state.needFocus = true;
            // 将当前文本拷贝到缓冲区
            strncpy(state.buf, text.c_str(), sizeof(state.buf) - 1);
            state.buf[sizeof(state.buf) - 1] = '\0';
        }
    } else {
        // ---- 编辑模式 ----
        // 给 InputText 一个不带 ## 的唯一 PushID，避免冲突
        ImGui::PushID(id);

        // 让输入框与 Label 宽度保持一致（可选：固定宽度或自适应）
        float textWidth = ImGui::CalcTextSize(state.buf).x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float minWidth  = 80.0f;
        ImGui::SetNextItemWidth(std::max(textWidth, minWidth));

        // 首次进入编辑模式时自动聚焦
        if (state.needFocus)
        {
            ImGui::SetKeyboardFocusHere();
            state.needFocus = false;
        }

        ImGuiInputTextFlags flags =
            ImGuiInputTextFlags_EnterReturnsTrue |   // Enter 提交
            ImGuiInputTextFlags_AutoSelectAll;        // 自动全选

        bool enterPressed = ImGui::InputText("##edit", state.buf, sizeof(state.buf), flags);

        bool lostFocus = !ImGui::IsItemActive() && !state.needFocus;
        bool escPressed = ImGui::IsKeyPressed(ImGuiKey_Escape);

        if (enterPressed || lostFocus)
        {
            // 提交新文本
            text = state.buf;
            state.isEditing = false;
            committed = true;
        }
        else if (escPressed)
        {
            // 取消，恢复原文本
            state.isEditing = false;
        }

        ImGui::PopID();
    }

    return committed;
}

namespace ImGui {

namespace {

struct ModalSuperState {
    float  flashTimer   = 0.0f;
    float  flashTotal   = 0.0f;
    float  shakeTimer   = 0.0f;
    float  shakeTotal   = 0.0f;
    float  cooldown     = 0.0f;
    bool   firstFrame   = true;
    bool   shaking      = false;
    ImVec2 shakeBasePos = ImVec2(0.0f, 0.0f);
};

std::unordered_map<ImGuiID, ModalSuperState> gModalSuperStates;

float calcModalHighlight(const ModalSuperState& state) {
    if(state.flashTimer <= 0.0f || state.flashTotal <= 0.0f) return 0.0f;
    const float progress = 1.0f - state.flashTimer / state.flashTotal; // 0 -> 1
    const float envelope = 1.0f - progress;
    const float pulse = 0.55f + 0.45f * std::sin(progress * 6.2831853f * 3.0f); // 3 pulses
    return envelope * pulse;
}

} // namespace

ModalSuperStyle& GetModalSuperStyle() {
    static ModalSuperStyle sModalSuperStyle;
    return sModalSuperStyle;
}

bool BeginPopupModalSuper(const char* name, bool* p_open, ImGuiWindowFlags flags) {
    ImGuiContext& g = *ImGui::GetCurrentContext();
    ImGuiWindow* parentWindow = g.CurrentWindow;
    IM_ASSERT(parentWindow != nullptr && "BeginPopupModalSuper() called outside of NewFrame()!");
    const ImGuiID id = parentWindow->GetID(name);
    const ModalSuperStyle& cfg = GetModalSuperStyle();

    if(!ImGui::IsPopupOpen(id, ImGuiPopupFlags_None)) {
        gModalSuperStates.erase(id);
        g.NextWindowData.ClearFlags();
        if(p_open && *p_open) *p_open = false;
        return false;
    }

    ModalSuperState& state = gModalSuperStates[id];

    // update timers
    const float dt = g.IO.DeltaTime;
    if(state.cooldown > 0.0f)   state.cooldown = (state.cooldown > dt) ? state.cooldown - dt : 0.0f;
    if(state.flashTimer > 0.0f) state.flashTimer = (state.flashTimer > dt) ? state.flashTimer - dt : 0.0f;
    if(state.shakeTimer > 0.0f) state.shakeTimer = (state.shakeTimer > dt) ? state.shakeTimer - dt : 0.0f;
    if(state.shakeTimer <= 0.0f) state.shaking = false;

    const float highlight = calcModalHighlight(state);

    if(state.shaking && state.shakeTimer > 0.0f && state.shakeTotal > 0.0f) {
        const float t = state.shakeTimer / state.shakeTotal; // 1 -> 0
        const float offset = std::sin((1.0f - t) * 3.14159265f * 6.0f) * cfg.shakeAmplitude * t;
        ImGui::SetNextWindowPos(ImVec2(state.shakeBasePos.x + offset, state.shakeBasePos.y), ImGuiCond_Always);
    }

    if(!ImGui::BeginPopupModal(name, p_open, flags)) return false;

    ImGuiWindow* modalWindow = ImGui::GetCurrentWindow();
    const ImVec2 windowPos = ImGui::GetWindowPos();
    const ImVec2 windowSize = ImGui::GetWindowSize();

    if(!state.firstFrame) {
        const ImVec2 mousePos = g.IO.MousePos;
        const bool mouseInside = mousePos.x >= windowPos.x && mousePos.y >= windowPos.y && mousePos.x <= windowPos.x + windowSize.x && mousePos.y <= windowPos.y + windowSize.y;

        ImGuiWindow* rawHovered = g.HoveredWindowBeforeClear;
        const bool hoveredOtherWindow = (rawHovered != nullptr) && (rawHovered != modalWindow) && !ImGui::IsWindowWithinBeginStackOf(rawHovered, modalWindow);

        bool attemptOutside = false;
        if(!mouseInside) {
            if(cfg.feedbackOnClickOutside) {
                for(int button = 0; button < 3 && !attemptOutside; ++button) {
                    if(ImGui::IsMouseClicked(button) && (rawHovered == nullptr || hoveredOtherWindow)) {
                        attemptOutside = true;
                    }
                }
            }
            if(!attemptOutside && cfg.feedbackOnHoverOutside && hoveredOtherWindow) {
                attemptOutside = true;
            }
        }

        if(attemptOutside && state.cooldown <= 0.0f) {
            state.flashTimer = state.flashTotal = (cfg.flashDuration > 0.0f) ? cfg.flashDuration : 0.01f;
            state.cooldown = cfg.cooldown;
            if(cfg.shakeWindow && !state.shaking) {
                state.shakeTimer = state.shakeTotal = (cfg.shakeDuration > 0.0f) ? cfg.shakeDuration : 0.01f;
                state.shakeBasePos = windowPos;
                state.shaking = true;
            }
            if(cfg.playSound) ::MessageBeep(cfg.beepType);
        }
    }
    state.firstFrame = false;

    // draw feedback effects
    if(highlight > 0.0f && (cfg.highlightBorder || cfg.flashWindow)) {
        ImDrawList* fgDrawList = ImGui::GetForegroundDrawList();
        const float rounding = g.Style.WindowRounding;
        const ImVec2 windowMax(windowPos.x + windowSize.x, windowPos.y + windowSize.y);

        if(cfg.flashWindow) {
            ImVec4 flashColor = cfg.highlightColor;
            flashColor.w *= highlight * 0.20f;
            fgDrawList->AddRectFilled(windowPos, windowMax, ImGui::GetColorU32(flashColor), rounding);
        }

        if(cfg.highlightBorder) {
            const float borderSize = (g.Style.WindowBorderSize > 0.0f) ? g.Style.WindowBorderSize : 1.0f;
            for(int layer = 3; layer >= 1; --layer) {
                const float expand = static_cast<float>(layer) * 1.5f;
                ImVec4 layerColor = cfg.highlightColor;
                layerColor.w *= highlight * 0.18f * static_cast<float>(4 - layer);
                fgDrawList->AddRect(ImVec2(windowPos.x - expand, windowPos.y - expand), ImVec2(windowMax.x + expand, windowMax.y + expand),
                                    ImGui::GetColorU32(layerColor), rounding + expand, 0, borderSize + static_cast<float>(layer) * 2.0f);
            }
            ImVec4 borderColor = cfg.highlightColor;
            borderColor.w *= highlight;
            fgDrawList->AddRect(windowPos, windowMax, ImGui::GetColorU32(borderColor), rounding, 0, borderSize + 2.0f);
        }
    }

    return true;
}

} // namespace ImGui
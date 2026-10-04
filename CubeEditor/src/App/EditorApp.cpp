#include "EditorApp.h"

#include <filesystem>
#include <fstream>
#include <system_error>
#include <vector>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "json.hpp"
#include "Cube/Core/Application.h"
#include "Cube/Core/Log.h"
#include "Cube/Event/ApplicationEvent.h"
#include "Cube/Renderer/Renderer.h"
#include "Cube/Core/Engine.h"

#include "../Project/Project.h"
#include "../Utils/misc.h"
#include "EditorPage.h"

extern Project* proj;

// TODO: 暂时硬编码路径
EditorApp::EditorApp(const Cube::WindowPros& windowPros) : Cube::Application(windowPros, {"D:/mycode/vsProject/CubeEngine/Cube/scripts"}) {
    imGuiInit();

    glfwSetDropCallback(mainWindow->getNativeWindow(), [](GLFWwindow* window, int path_count, const char* paths[]) {
        Page* curPage = EditorApp::get()->currentPage.get();
        if(curPage && curPage->getType() == Page::Type::Editor) {
            for(int i = 0; i < path_count; ++i) {
                static_cast<EditorPage*>(curPage)->getProject()->importResource(Cube::Path(paths[i]));
            }
        }
    });
}

EditorApp::~EditorApp() {
    // destroy current page before shutting down ImGui, because EditorPage destructor needs ImGui context.
    currentPage.reset();
    ImGui_ImplGlfw_Shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui::DestroyContext();
}


void EditorApp::switchPage(Page* page) {
    currentPage.reset(page);
}

void EditorApp::run() {
    running = true;
    std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
    while(running) {
        std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> frameDuration = currentTime - lastTime;
        lastTime = currentTime;
        float deltaTime = frameDuration.count();

        Cube::Renderer::clearBuffer();
        if(currentPage) {
            currentPage->render(deltaTime);
        }
        mainWindow->update();
    }
}

Cube::Path EditorApp::getConfigDir() {
    Cube::Path configDir = Utils::getUserConfigDir() / "CubeEditor"; // TODO: 改成域名倒写
    std::error_code ec;
    std::filesystem::create_directories(configDir.fspath(), ec);
    if (ec) {
        CB_EDITOR_ERROR("Failed to create directory: {}, {}", configDir, ec.message());
        return Cube::Path();
    }
    return configDir;
}

void EditorApp::imGuiInit() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io.Fonts->AddFontFromFileTTF("assets/fonts/SourceHanSansSC/SourceHanSansSC-Normal.otf", 30.0f);
    // TODO: 提供一个ImGui预设布局，当没有imgui.ini时加载这个预设布局

    ImGuiStyle& style = ImGui::GetStyle();
    style.HoverDelayNormal = 0.8f;
    style.HoverDelayShort = 0.5f;

    setDarkTheme();

    ImGui_ImplGlfw_InitForOpenGL(mainWindow->getNativeWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330 core");
}

void EditorApp::setDarkTheme() {
    if(true) {
        // deepseek dark v3
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // ====================================================
        // 1. 增强的色调方案 - 更高的对比度与更明确的交互反馈
        // ====================================================

        // 基础背景色 (更深的背景增强内容层次)
        colors[ImGuiCol_WindowBg] = ImVec4(0.08f, 0.09f, 0.11f, 0.95f);  // #141618
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.11f, 0.13f, 0.90f);   // #1A1C21
        colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.12f, 0.15f, 1.0f);    // #1C1F26

        // 文本颜色 - 更高的对比度
        colors[ImGuiCol_Text] = ImVec4(0.96f, 0.97f, 0.99f, 1.00f);          // #F5F8FC
        colors[ImGuiCol_TextDisabled] = ImVec4(0.45f, 0.46f, 0.48f, 1.00f);  // #73767A

        // 边框和分隔线 - 更清晰的分隔
        colors[ImGuiCol_Border] = ImVec4(0.28f, 0.30f, 0.33f, 0.60f);     // #474D54
        colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.32f, 0.35f, 0.65f);  // #4D5259

        // ====================================================
        // 2. 强化交互反馈的控件颜色方案
        // ====================================================

        // 框架背景 (更明显的悬停/激活状态)
        colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.17f, 0.20f, 0.80f);         // #262B33
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.23f, 0.27f, 0.80f);  // #333B43
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.28f, 0.32f, 0.80f);   // #404752

        // 按钮 - 更强烈的交互反馈
        colors[ImGuiCol_Button] = ImVec4(0.18f, 0.38f, 0.65f, 0.90f);         // #2E61A6
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.25f, 0.48f, 0.80f, 1.00f);  // #407AD0
        colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.32f, 0.55f, 1.00f);   // #26528C

        // 页签 - 未选中与标签栏同色(融入背景)，选中用更深的底色区分
        // 注意: dock 区域通常未聚焦，ImGui 会用 TabDimmed/TabDimmedSelected，必须一并设置；
        //       标签栏底色与节点是否聚焦有关(见 TitleBg/TitleBgActive)，两套颜色要分别对齐
        colors[ImGuiCol_Tab] = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);                       // = TitleBgActive
        colors[ImGuiCol_TabDimmed] = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);                 // = TitleBg
        colors[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.21f, 0.26f, 1.00f);                // #2E3642
        colors[ImGuiCol_TabSelected] = ImVec4(0.05f, 0.06f, 0.07f, 1.00f);               // 选中(更深)
        colors[ImGuiCol_TabDimmedSelected] = ImVec4(0.05f, 0.06f, 0.07f, 1.00f);         // 选中(更深)
        colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.25f, 0.65f, 1.00f, 1.00f);       // #40A5FF
        colors[ImGuiCol_TabDimmedSelectedOverline] = ImVec4(0.25f, 0.65f, 1.00f, 1.00f); // #40A5FF

        // 标题栏 - 更强的视觉焦点
        colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.11f, 0.13f, 1.00f);           // #1A1C21
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.14f, 0.16f, 0.19f, 1.00f);     // #242930
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.11f, 0.13f, 0.75f);  // #1A1C21

        // ====================================================
        // 3. 强化交互元素反馈
        // ====================================================

        // 选中状态 - 更明显的视觉反馈
        colors[ImGuiCol_Header] = ImVec4(0.22f, 0.45f, 0.75f, 0.50f);         // #3873BF
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.48f, 0.80f, 0.65f);  // #407AD0
        colors[ImGuiCol_HeaderActive] = ImVec4(0.18f, 0.38f, 0.65f, 0.80f);   // #2E61A6

        // 复选框/单选按钮 - 更鲜明的选中状态
        colors[ImGuiCol_CheckMark] = ImVec4(0.25f, 0.65f, 1.00f, 1.00f);  // #40A5FF

        // 滑块 - 更强的抓取点视觉反馈
        colors[ImGuiCol_SliderGrab] = ImVec4(0.25f, 0.65f, 1.00f, 0.80f);        // #40A5FF
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.75f, 1.00f, 1.00f);  // #59BFFF

        // ====================================================
        // 4. 视觉增强元素
        // ====================================================

        // 强调色 - 更鲜明的蓝色作为主强调色
        ImVec4 accent_color = ImVec4(0.25f, 0.65f, 1.00f, 1.00f);  // #40A5FF

        // 添加辅助强调色 (绿色用于积极操作)
        ImVec4 positive_color = ImVec4(0.20f, 0.75f, 0.45f, 1.00f);  // #33BF73

        // 应用强调色到关键元素
        colors[ImGuiCol_CheckMark] = accent_color;
        colors[ImGuiCol_SliderGrab] = accent_color;
        colors[ImGuiCol_PlotLines] = accent_color;
        colors[ImGuiCol_NavHighlight] = accent_color;

        // 菜单栏
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.14f, 0.16f, 1.00f);  // #1F2326

        // 滚动条 - 更明显的抓取点
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.11f, 0.13f, 0.60f);           // #1A1C21
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.37f, 0.40f, 0.80f);         // #595E66
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.42f, 0.45f, 0.90f);  // #666B73
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.47f, 0.50f, 1.00f);   // #737880

        // 调整大小手柄 - 更明显
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.40f, 0.42f, 0.45f, 0.70f);  // #666B73
        colors[ImGuiCol_ResizeGripHovered] = accent_color;
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.45f, 0.47f, 0.50f, 1.00f);  // #737880

        // 绘图元素 - 使用强调色
        colors[ImGuiCol_PlotLinesHovered] = positive_color;
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.20f, 1.00f);         // #E6B333
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.80f, 0.30f, 1.00f);  // #FFCC4D

        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.0, 1.0f, 1.0f, 0.0f);
        // ====================================================
        // 5. 增强的样式参数 (更清晰的视觉层次)
        // ====================================================
        style.TabBarOverlineSize = 2.0f; // 选中页签顶部的高亮条宽度
        // 圆角效果 - 稍微减小圆角使界面更有力量感
        style.WindowRounding = 7.0f;
        style.ChildRounding = 6.0f;
        style.FrameRounding = 5.0f;
        style.PopupRounding = 7.0f;
        style.ScrollbarRounding = 6.0f;
        style.GrabRounding = 4.0f;
        style.TabRounding = 0.0f;

        // 控件尺寸 - 增加尺寸强化存在感
        style.ScrollbarSize = 18;
        style.GrabMinSize = 14;

        // 窗口按钮位置
        style.WindowMenuButtonPosition = ImGuiDir_None;

        // 对齐方式
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    }
    /*
    if(false) {
        // deepseek light v1
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // ====================================================
        // 1. Windows 11 核心颜色方案
        // ====================================================

        // 基础背景色 (Windows 11 浅色模式背景)
        colors[ImGuiCol_WindowBg] = ImVec4(0.95f, 0.95f, 0.96f, 1.00f);  // #F2F2F6
        colors[ImGuiCol_ChildBg] = ImVec4(0.97f, 0.97f, 0.98f, 1.00f);   // #F7F7FA
        colors[ImGuiCol_PopupBg] = ImVec4(0.98f, 0.98f, 0.99f, 1.00f);   // #FAFAFC

        // 文本颜色
        colors[ImGuiCol_Text] = ImVec4(0.13f, 0.13f, 0.15f, 1.00f);          // #222226
        colors[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.62f, 1.00f);  // #99999F

        // 边框和分隔线 (Windows 11 风格细边框)
        colors[ImGuiCol_Border] = ImVec4(0.85f, 0.85f, 0.87f, 1.00f);  // #D9D9DE
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.85f, 0.85f, 0.87f, 1.00f);  // #D9D9DE

        // ====================================================
        // 2. Windows 11 控件颜色方案
        // ====================================================

        // 框架背景 (输入框、滑块背景等)
        colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);         // #FFFFFF
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.97f, 0.97f, 0.98f, 1.00f);  // #F7F7FA
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.95f, 0.95f, 0.96f, 1.00f);   // #F2F2F6

        // 按钮 (Windows 11 风格的柔和按钮)
        colors[ImGuiCol_Button] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);         // #FFFFFF
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.95f, 0.95f, 0.96f, 1.00f);  // #F2F2F6
        colors[ImGuiCol_ButtonActive] = ImVec4(0.93f, 0.93f, 0.94f, 1.00f);   // #EDEDF0

        // 页签 (圆角标签设计)
        colors[ImGuiCol_Tab] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);                 // #FFFFFF
        colors[ImGuiCol_TabHovered] = ImVec4(0.95f, 0.95f, 0.96f, 1.00f);          // #F2F2F6
        colors[ImGuiCol_TabActive] = ImVec4(0.97f, 0.97f, 0.98f, 1.00f);           // #F7F7FA
        colors[ImGuiCol_TabUnfocused] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);        // #FFFFFF
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.97f, 0.97f, 0.98f, 1.00f);  // #F7F7FA

        // 标题栏 (Windows 11 风格标题栏)
        colors[ImGuiCol_TitleBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);           // #FFFFFF
        colors[ImGuiCol_TitleBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);     // #FFFFFF
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);  // #FFFFFF

        // ====================================================
        // 3. Windows 11 强调色与交互元素
        // ====================================================

        // Windows 11 默认强调色 (柔和的蓝色)
        ImVec4 accent_color = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);  // #0078D4

        // 选中状态
        colors[ImGuiCol_Header] = ImVec4(accent_color.x, accent_color.y, accent_color.z, 0.15f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(accent_color.x, accent_color.y, accent_color.z, 0.25f);
        colors[ImGuiCol_HeaderActive] = ImVec4(accent_color.x, accent_color.y, accent_color.z, 0.35f);

        // 复选框/单选按钮
        colors[ImGuiCol_CheckMark] = accent_color;

        // 滑块
        colors[ImGuiCol_SliderGrab] = accent_color;
        colors[ImGuiCol_SliderGrabActive] = ImVec4(accent_color.x * 0.8f, accent_color.y * 0.8f, accent_color.z * 0.8f, 1.00f);

        // 菜单栏
        colors[ImGuiCol_MenuBarBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);  // #FFFFFF

        // 滚动条
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.95f, 0.95f, 0.96f, 0.00f);           // #F2F2F6
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.82f, 0.60f);         // #CCCCD1
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.75f, 0.75f, 0.77f, 0.80f);  // #BFBFC4
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.70f, 0.70f, 0.72f, 1.00f);   // #B3B3B8

        // 调整大小手柄
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.80f, 0.80f, 0.82f, 0.60f);  // #CCCCD1
        colors[ImGuiCol_ResizeGripHovered] = accent_color;
        colors[ImGuiCol_ResizeGripActive] = ImVec4(accent_color.x * 0.8f, accent_color.y * 0.8f, accent_color.z * 0.8f, 1.00f);

        // 绘图元素
        colors[ImGuiCol_PlotLines] = accent_color;
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.20f, 0.75f, 0.45f, 1.00f);      // #33BF73 (绿色)
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.20f, 1.00f);         // #E6B333 (琥珀色)
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.95f, 0.80f, 0.30f, 1.00f);  // #F2CC4D

        // ====================================================
        // 4. Windows 11 样式参数 (圆角、间距等)
        // ====================================================

        // 圆角效果 (Windows 11 特色圆角)
        style.WindowRounding = 8.0f;     // 窗口圆角
        style.ChildRounding = 8.0f;      // 子窗口圆角
        style.FrameRounding = 6.0f;      // 控件圆角 (按钮等)
        style.PopupRounding = 8.0f;      // 弹出窗口圆角
        style.ScrollbarRounding = 8.0f;  // 滚动条圆角
        style.GrabRounding = 6.0f;       // 滑块抓取点圆角
        style.TabRounding = 8.0f;        // 标签圆角

        // 间距和内边距 (Windows 11 现代间距)
        style.WindowPadding = ImVec2(12, 12);    // 窗口内边距
        style.FramePadding = ImVec2(12, 8);      // 控件内边距
        style.CellPadding = ImVec2(8, 4);        // 表格单元格内边距
        style.ItemSpacing = ImVec2(12, 10);      // 项之间的水平/垂直间距
        style.ItemInnerSpacing = ImVec2(8, 6);   // 项内部元素间距
        style.TouchExtraPadding = ImVec2(0, 0);  // 触摸操作的额外内边距
        style.IndentSpacing = 24;                // 缩进宽度

        // 边框尺寸 (Windows 11 细边框)
        style.WindowBorderSize = 1.0f;  // 窗口边框
        style.ChildBorderSize = 1.0f;   // 子窗口边框
        style.PopupBorderSize = 1.0f;   // 弹出窗口边框
        style.FrameBorderSize = 1.0f;   // 框架边框
        style.TabBorderSize = 1.0f;     // 标签边框

        // 控件尺寸
        style.ScrollbarSize = 16;  // 滚动条宽度
        style.GrabMinSize = 12;    // 滑块抓取点最小尺寸

        // 窗口按钮位置 (Windows 11 风格)
        style.WindowMenuButtonPosition = ImGuiDir_Right;  // 窗口菜单按钮在右侧

        // 对齐方式
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);  // 窗口标题对齐 (左对齐居中)
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);   // 按钮文本居中
    }
    if(false) {
        // deepseek VSCode v1
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // ====================================================
        // 1. VS Code 核心颜色方案
        // ====================================================

        // 基础背景色 (VS Code 深色主题背景)
        colors[ImGuiCol_WindowBg] = ImVec4(0.156f, 0.156f, 0.156f, 0.98f);  // #282828
        colors[ImGuiCol_ChildBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);      // #1C1C1C
        colors[ImGuiCol_PopupBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.98f);      // #333333

        // 文本颜色
        colors[ImGuiCol_Text] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);          // #D1D1D1
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);  // #808080

        // 边框和分隔线
        colors[ImGuiCol_Border] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);  // #454545
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.27f, 0.27f, 0.27f, 1.00f);  // #454545

        // ====================================================
        // 2. VS Code 控件颜色方案
        // ====================================================

        // 框架背景 (输入框、滑块背景等)
        colors[ImGuiCol_FrameBg] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);         // #383838
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);  // #474747
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);   // #404040

        // 按钮
        colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);         // #383838
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);  // #4D4D4D
        colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);   // #333333

        // 页签
        colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);                 // #262626
        colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);          // #333333
        colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);           // #383838
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);        // #262626
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);  // #2E2E2E

        // 标题栏
        colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);           // #1A1A1A
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);     // #1A1A1A
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.10f, 0.10f, 0.75f);  // #1A1A1A

        // ====================================================
        // 3. VS Code 强调色与交互元素
        // ====================================================

        // VS Code 默认强调色 (蓝色)
        ImVec4 accent_color = ImVec4(0.00f, 0.47f, 0.84f, 1.00f);  // #0078D4

        // 选中状态
        colors[ImGuiCol_Header] = ImVec4(0.17f, 0.36f, 0.53f, 1.00f);         // #2B5C87 (比强调色深)
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.20f, 0.40f, 0.60f, 1.00f);  // #336699
        colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.32f, 0.48f, 1.00f);   // #265280

        // 复选框/单选按钮
        colors[ImGuiCol_CheckMark] = accent_color;

        // 滑块
        colors[ImGuiCol_SliderGrab] = accent_color;
        colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 0.35f, 0.63f, 1.00f);  // #0059A0

        // 菜单栏
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);  // #262626

        // 滚动条
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.11f, 0.11f, 0.11f, 0.00f);           // #1C1C1C
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.35f, 0.35f, 0.80f);         // #595959
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.80f);  // #666666
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);   // #737373

        // 调整大小手柄
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.35f, 0.35f, 0.60f);  // #595959
        colors[ImGuiCol_ResizeGripHovered] = accent_color;
        colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 0.35f, 0.63f, 1.00f);  // #0059A0

        // 绘图元素
        colors[ImGuiCol_PlotLines] = accent_color;
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.20f, 0.75f, 0.45f, 1.00f);      // #33BF73 (绿色)
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.20f, 1.00f);         // #E6B333 (琥珀色)
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.80f, 0.30f, 1.00f);  // #FFCC4D

        // ====================================================
        // 4. VS Code 样式参数
        // ====================================================

        // 圆角效果 (VS Code 使用少量圆角)
        style.WindowRounding = 4.0f;     // 窗口圆角
        style.ChildRounding = 4.0f;      // 子窗口圆角
        style.FrameRounding = 2.0f;      // 控件圆角 (按钮等)
        style.PopupRounding = 4.0f;      // 弹出窗口圆角
        style.ScrollbarRounding = 4.0f;  // 滚动条圆角
        style.GrabRounding = 2.0f;       // 滑块抓取点圆角
        style.TabRounding = 4.0f;        // 标签圆角

        // 间距和内边距
        style.WindowPadding = ImVec2(8, 8);      // 窗口内边距
        style.FramePadding = ImVec2(6, 4);       // 控件内边距
        style.CellPadding = ImVec2(4, 2);        // 表格单元格内边距
        style.ItemSpacing = ImVec2(8, 6);        // 项之间的水平/垂直间距
        style.ItemInnerSpacing = ImVec2(4, 4);   // 项内部元素间距
        style.TouchExtraPadding = ImVec2(0, 0);  // 触摸操作的额外内边距
        style.IndentSpacing = 22;                // 缩进宽度

        // 边框尺寸
        style.WindowBorderSize = 1.0f;  // 窗口边框
        style.ChildBorderSize = 1.0f;   // 子窗口边框
        style.PopupBorderSize = 1.0f;   // 弹出窗口边框
        style.FrameBorderSize = 1.0f;   // 框架边框
        style.TabBorderSize = 0.0f;     // 标签边框

        // 控件尺寸
        style.ScrollbarSize = 14;  // 滚动条宽度
        style.GrabMinSize = 10;    // 滑块抓取点最小尺寸

        // 窗口按钮位置
        style.WindowMenuButtonPosition = ImGuiDir_Left;

        // 对齐方式
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);  // 窗口标题对齐 (左对齐居中)
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);   // 按钮文本居中
    }
    if(false) {
        // deepseek light pink v1
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // ====================================================
        // 1. 核心颜色方案 - 粉色为主色调
        // ====================================================

        // 基础背景色 (柔和的浅粉色)
        colors[ImGuiCol_WindowBg] = ImVec4(0.98f, 0.95f, 0.96f, 0.98f);  // #FAF0F5
        colors[ImGuiCol_ChildBg] = ImVec4(0.99f, 0.97f, 0.98f, 0.95f);   // #FDF8FA
        colors[ImGuiCol_PopupBg] = ImVec4(1.00f, 0.98f, 0.99f, 0.99f);   // #FFFCFD

        // 文本颜色
        colors[ImGuiCol_Text] = ImVec4(0.27f, 0.20f, 0.27f, 1.00f);          // #453345
        colors[ImGuiCol_TextDisabled] = ImVec4(0.65f, 0.58f, 0.65f, 1.00f);  // #A695A6

        // 边框和分隔线
        colors[ImGuiCol_Border] = ImVec4(0.93f, 0.85f, 0.90f, 0.60f);  // #EDD9E6
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.93f, 0.85f, 0.90f, 0.60f);  // #EDD9E6

        // ====================================================
        // 2. 控件颜色方案
        // ====================================================

        // 框架背景
        colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);         // #FFFFFF
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.99f, 0.96f, 0.98f, 1.00f);  // #FDF5FA
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.98f, 0.94f, 0.97f, 1.00f);   // #FAF1F7

        // 按钮
        colors[ImGuiCol_Button] = ImVec4(0.98f, 0.85f, 0.92f, 1.00f);         // #FAD9EB
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.96f, 0.78f, 0.89f, 1.00f);  // #F5C7E3
        colors[ImGuiCol_ButtonActive] = ImVec4(0.94f, 0.70f, 0.85f, 1.00f);   // #F0B2D9

        // 页签
        colors[ImGuiCol_Tab] = ImVec4(0.98f, 0.92f, 0.96f, 0.85f);                 // #FAEBF5
        colors[ImGuiCol_TabHovered] = ImVec4(0.96f, 0.85f, 0.92f, 0.90f);          // #F5D9EB
        colors[ImGuiCol_TabActive] = ImVec4(0.98f, 0.85f, 0.92f, 1.00f);           // #FAD9EB
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.98f, 0.95f, 0.97f, 0.85f);        // #FAF2F7
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.98f, 0.90f, 0.95f, 0.95f);  // #FAE6F2

        // 标题栏
        colors[ImGuiCol_TitleBg] = ImVec4(0.98f, 0.92f, 0.96f, 0.90f);           // #FAEBF5
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.98f, 0.85f, 0.92f, 0.95f);     // #FAD9EB
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.98f, 0.92f, 0.96f, 0.80f);  // #FAEBF5

        // ====================================================
        // 3. 强调色与交互元素
        // ====================================================

        // 主强调色 (柔和的粉色)
        ImVec4 accent_color = ImVec4(0.94f, 0.50f, 0.70f, 1.00f);  // #F080B3

        // 辅助强调色 (紫粉色)
        ImVec4 secondary_accent = ImVec4(0.80f, 0.50f, 0.80f, 1.00f);  // #CC80CC

        // 选中状态
        colors[ImGuiCol_Header] = ImVec4(accent_color.x, accent_color.y, accent_color.z, 0.25f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(accent_color.x, accent_color.y, accent_color.z, 0.35f);
        colors[ImGuiCol_HeaderActive] = ImVec4(accent_color.x, accent_color.y, accent_color.z, 0.45f);

        // 复选框/单选按钮
        colors[ImGuiCol_CheckMark] = accent_color;

        // 滑块
        colors[ImGuiCol_SliderGrab] = accent_color;
        colors[ImGuiCol_SliderGrabActive] = ImVec4(accent_color.x * 0.9f, accent_color.y * 0.9f, accent_color.z * 0.9f, 1.00f);

        // 菜单栏
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.98f, 0.95f, 0.97f, 0.95f);  // #FAF2F7

        // 滚动条
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.95f, 0.97f, 0.00f);           // #FAF2F7
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.93f, 0.85f, 0.90f, 0.60f);         // #EDD9E6
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.90f, 0.80f, 0.88f, 0.80f);  // #E6CCE0
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.88f, 0.75f, 0.85f, 1.00f);   // #E0BFD9

        // 调整大小手柄
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.93f, 0.85f, 0.90f, 0.60f);  // #EDD9E6
        colors[ImGuiCol_ResizeGripHovered] = secondary_accent;
        colors[ImGuiCol_ResizeGripActive] = ImVec4(secondary_accent.x * 0.9f, secondary_accent.y * 0.9f, secondary_accent.z * 0.9f, 1.00f);

        // 绘图元素
        colors[ImGuiCol_PlotLines] = accent_color;
        colors[ImGuiCol_PlotLinesHovered] = secondary_accent;
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.80f, 1.00f);         // #E6B3CC
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.85f, 0.60f, 0.75f, 1.00f);  // #D999BF

        // ====================================================
        // 4. 样式参数 (圆角、间距等)
        // ====================================================

        // 圆角效果
        style.WindowRounding = 8.0f;     // 窗口圆角
        style.ChildRounding = 8.0f;      // 子窗口圆角
        style.FrameRounding = 6.0f;      // 控件圆角
        style.PopupRounding = 8.0f;      // 弹出窗口圆角
        style.ScrollbarRounding = 8.0f;  // 滚动条圆角
        style.GrabRounding = 6.0f;       // 滑块抓取点圆角
        style.TabRounding = 8.0f;        // 标签圆角

        // 间距和内边距
        style.WindowPadding = ImVec2(12, 12);    // 窗口内边距
        style.FramePadding = ImVec2(10, 6);      // 控件内边距
        style.CellPadding = ImVec2(8, 5);        // 表格单元格内边距
        style.ItemSpacing = ImVec2(12, 10);      // 项之间的间距
        style.ItemInnerSpacing = ImVec2(8, 6);   // 项内部元素间距
        style.TouchExtraPadding = ImVec2(0, 0);  // 触摸操作的额外内边距
        style.IndentSpacing = 24;                // 缩进宽度

        // 边框尺寸
        style.WindowBorderSize = 1.0f;  // 窗口边框
        style.ChildBorderSize = 1.0f;   // 子窗口边框
        style.PopupBorderSize = 1.0f;   // 弹出窗口边框
        style.FrameBorderSize = 0.0f;   // 框架边框
        style.TabBorderSize = 0.0f;     // 标签边框

        // 控件尺寸
        style.ScrollbarSize = 16;  // 滚动条宽度
        style.GrabMinSize = 12;    // 滑块抓取点最小尺寸

        // 窗口按钮位置
        style.WindowMenuButtonPosition = ImGuiDir_Right;

        // 对齐方式
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);  // 窗口标题对齐
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);   // 按钮文本居中
    }
    if(false) {
        // deepseek dark pink v1
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors = style.Colors;

        // ====================================================
        // 1. 核心颜色方案 - 深色背景搭配粉色点缀
        // ====================================================

        // 基础背景色 (深灰紫色)
        colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.10f, 0.14f, 0.98f);  // #1F1A24
        colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.08f, 0.12f, 0.95f);   // #1A1419
        colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.12f, 0.18f, 0.99f);   // #261F2E

        // 文本颜色 (柔和的浅紫色)
        colors[ImGuiCol_Text] = ImVec4(0.92f, 0.88f, 0.94f, 1.00f);          // #EBE0F0
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.45f, 0.55f, 1.00f);  // #80738C

        // 边框和分隔线 (深紫色)
        colors[ImGuiCol_Border] = ImVec4(0.25f, 0.20f, 0.28f, 0.60f);  // #403347
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_Separator] = ImVec4(0.30f, 0.25f, 0.35f, 0.60f);  // #4D4059

        // ====================================================
        // 2. 控件颜色方案 - 粉色作为强调色
        // ====================================================

        // 框架背景
        colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.15f, 0.21f, 1.00f);         // #2E2635
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.20f, 0.30f, 1.00f);  // #40334D
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.22f, 0.18f, 0.26f, 1.00f);   // #382E40

        // 按钮 (粉色渐变)
        colors[ImGuiCol_Button] = ImVec4(0.60f, 0.30f, 0.50f, 0.80f);         // #994D80
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.70f, 0.35f, 0.60f, 0.90f);  // #B35999
        colors[ImGuiCol_ButtonActive] = ImVec4(0.65f, 0.25f, 0.55f, 1.00f);   // #A6408C

        // 页签
        colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.15f, 0.21f, 0.85f);                 // #2E2635
        colors[ImGuiCol_TabHovered] = ImVec4(0.25f, 0.20f, 0.30f, 0.90f);          // #40334D
        colors[ImGuiCol_TabActive] = ImVec4(0.35f, 0.25f, 0.40f, 1.00f);           // #594066
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.12f, 0.18f, 0.85f);        // #261F2E
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.16f, 0.24f, 0.95f);  // #33293D

        // 标题栏
        colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.12f, 0.18f, 0.90f);           // #261F2E
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.20f, 0.15f, 0.25f, 0.95f);     // #332640
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.12f, 0.18f, 0.80f);  // #261F2E

        // ====================================================
        // 3. 强调色与交互元素 - 粉色为主
        // ====================================================

        // 主强调色 (霓虹粉色)
        ImVec4 accent_pink = ImVec4(1.00f, 0.40f, 0.80f, 1.00f);  // #FF66CC

        // 辅助强调色 (紫罗兰色)
        ImVec4 secondary_purple = ImVec4(0.80f, 0.40f, 1.00f, 1.00f);  // #CC66FF

        // 选中状态
        colors[ImGuiCol_Header] = ImVec4(accent_pink.x, accent_pink.y, accent_pink.z, 0.30f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(accent_pink.x, accent_pink.y, accent_pink.z, 0.40f);
        colors[ImGuiCol_HeaderActive] = ImVec4(accent_pink.x, accent_pink.y, accent_pink.z, 0.50f);

        // 复选框/单选按钮
        colors[ImGuiCol_CheckMark] = accent_pink;

        // 滑块
        colors[ImGuiCol_SliderGrab] = accent_pink;
        colors[ImGuiCol_SliderGrabActive] = ImVec4(accent_pink.x * 0.9f, accent_pink.y * 0.9f, accent_pink.z * 0.9f, 1.00f);

        // 菜单栏
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.12f, 0.18f, 0.95f);  // #261F2E

        // 滚动条
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.08f, 0.12f, 0.00f);           // #1A1419
        colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.35f, 0.25f, 0.40f, 0.60f);         // #594066
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.45f, 0.30f, 0.50f, 0.80f);  // #734D80
        colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50f, 0.35f, 0.55f, 1.00f);   // #80598C

        // 调整大小手柄
        colors[ImGuiCol_ResizeGrip] = ImVec4(0.35f, 0.25f, 0.40f, 0.60f);  // #594066
        colors[ImGuiCol_ResizeGripHovered] = secondary_purple;
        colors[ImGuiCol_ResizeGripActive] = ImVec4(secondary_purple.x * 0.9f, secondary_purple.y * 0.9f, secondary_purple.z * 0.9f, 1.00f);

        // 绘图元素
        colors[ImGuiCol_PlotLines] = accent_pink;
        colors[ImGuiCol_PlotLinesHovered] = secondary_purple;
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.80f, 0.40f, 0.60f, 1.00f);         // #CC6699
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.90f, 0.50f, 0.70f, 1.00f);  // #E680B3

        // ====================================================
        // 4. 样式参数 (圆角、间距等)
        // ====================================================

        // 圆角效果
        style.WindowRounding = 8.0f;     // 窗口圆角
        style.ChildRounding = 8.0f;      // 子窗口圆角
        style.FrameRounding = 6.0f;      // 控件圆角
        style.PopupRounding = 8.0f;      // 弹出窗口圆角
        style.ScrollbarRounding = 8.0f;  // 滚动条圆角
        style.GrabRounding = 6.0f;       // 滑块抓取点圆角
        style.TabRounding = 8.0f;        // 标签圆角

        // 间距和内边距
        style.WindowPadding = ImVec2(12, 12);    // 窗口内边距
        style.FramePadding = ImVec2(10, 6);      // 控件内边距
        style.CellPadding = ImVec2(8, 5);        // 表格单元格内边距
        style.ItemSpacing = ImVec2(12, 10);      // 项之间的间距
        style.ItemInnerSpacing = ImVec2(8, 6);   // 项内部元素间距
        style.TouchExtraPadding = ImVec2(0, 0);  // 触摸操作的额外内边距
        style.IndentSpacing = 24;                // 缩进宽度

        // 边框尺寸
        style.WindowBorderSize = 1.0f;  // 窗口边框
        style.ChildBorderSize = 1.0f;   // 子窗口边框
        style.PopupBorderSize = 1.0f;   // 弹出窗口边框
        style.FrameBorderSize = 0.0f;   // 框架边框
        style.TabBorderSize = 0.0f;     // 标签边框

        // 控件尺寸
        style.ScrollbarSize = 16;  // 滚动条宽度
        style.GrabMinSize = 12;    // 滑块抓取点最小尺寸

        // 窗口按钮位置
        style.WindowMenuButtonPosition = ImGuiDir_Right;

        // 对齐方式
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);  // 窗口标题对齐
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);   // 按钮文本居中
    }
    */
}

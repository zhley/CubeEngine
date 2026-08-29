#include "GuidancePage.h"

#include <filesystem>
#include <fstream>
#include <string>

#include "Cube/Core/Log.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Cube/Core/Engine.h"

#include "../Project/Project.h"
#include "../Utils/ImGuiExternal.h"
#include "../Utils/FileDialog.h"
#include "EditorApp.h"
#include "EditorPage.h"

GuidancePage::GuidancePage() {
    // load project path cache
    std::filesystem::path projectPathCacheFile = static_cast<EditorApp*>(Cube::Engine::getApp())->getConfigDir() / "projects_path_cache.json";
    std::ifstream file(projectPathCacheFile);
    if (!file.is_open()) {
        if (!std::filesystem::exists(projectPathCacheFile)) {
            return;
        }
        CB_EDITOR_ERROR("GuidancePage::GuidancePage: Failed to open file: {}", projectPathCacheFile.string());
        return;
    }
    nlohmann::json data;
    file >> data;
    file.close();
    projectsPathCache = data;
}

GuidancePage::~GuidancePage() {
    // save project path cache
    std::filesystem::path projectPathCacheFile = static_cast<EditorApp*>(Cube::Engine::getApp())->getConfigDir() / "projects_path_cache.json";
    std::ofstream file(projectPathCacheFile);
    if (!file.is_open()) {
        CB_EDITOR_ERROR("GuidancePage::~GuidancePage: Failed to open file: {}", projectPathCacheFile.string());
        return;
    }
    nlohmann::json data = projectsPathCache;
    file << data;
    file.close();
}

void GuidancePage::render(float deltaTime) {
    static Project* proj = nullptr;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->Flags);

    ImGui::Begin("Guidance");

    ImGui::BeginGroup();
    bool switchPage = false;
    auto toRemoveIt = projectsPathCache.end();
    for(auto it = projectsPathCache.begin(); it != projectsPathCache.end(); ++it) {
        const std::string& p = *it;
        if(ImGui::Button(p.c_str())) {
            proj = new Project(p);
            switchPage = true;
        }
        ImGui::SameLine();
        if(ImGui::Button((std::string("X##") + p).c_str())) {
            toRemoveIt = it;
        }
    }
    if(toRemoveIt != projectsPathCache.end()) {
        projectsPathCache.erase(toRemoveIt);
    }
    ImGui::EndGroup();

    static bool isNameValid = true;
    static bool isPathValid = true;
    static char name[50] = {};
    static char path[256] = {};
    static std::unique_ptr<ModalPopup> newProject = std::make_unique<ModalPopup>("New Project", [] {
        ImGui::Text("Project Name:");
        if(!isNameValid) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("This name is not valid!");
            ImGui::PopStyleColor();
        }
        ImGui::InputText("##ProjectName", name, IM_ARRAYSIZE(name));

        ImGui::Text("Project Path:");
        if(!isPathValid) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
            ImGui::Text("This path is not valid!");
            ImGui::PopStyleColor();
        }
        ImGui::InputText("##ProjectPath", path, IM_ARRAYSIZE(path));
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, toImColor({70, 77, 88, 255}));
        if(ImGui::Button("...")) {
            strcpy_s(path, Utils::FileDialog::selectDir().c_str());
        }
        ImGui::PopStyleColor();
    }, [&switchPage, this] {
        isNameValid = !std::string(name).empty();
        isPathValid = std::filesystem::exists(path);
        if(isNameValid && isPathValid){
            delete proj;
            projectsPathCache.push_back(std::string(path) + "/" + name + ".cbproj");
            proj = new Project(name, path);
            newProject->close();
            ImGui::CloseCurrentPopup();

            switchPage = true;
        }
    }, [] {
        isNameValid = true;
        isPathValid = true;
        memset(name, '\0', sizeof(name));
        memset(path, '\0', sizeof(path));
    });

    constexpr ImVec2 buttonSize = {128, 128};
    ImGui::SetCursorPos({ImGui::GetWindowWidth() / 2 - (buttonSize.x * 2 + 100 + ImGui::GetStyle().FramePadding.x * 2 * 2) / 2, 300.0f});
    ImGui::BeginGroup();
    ImGui::PushStyleColor(ImGuiCol_Button, toImColor({70, 77, 88, 255}));

    ImGui::BeginGroup();
    if(ImGui::ImageButton("New Project##1", new_project_png->getId(), buttonSize, {0, 1}, {1, 0})) {
        newProject->open();
    }
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (buttonSize.x + ImGui::GetStyle().FramePadding.x * 2) / 2 - ImGui::CalcTextSize("New Project").x / 2);
    ImGui::Text("New Project");
    ImGui::EndGroup();

    ImGui::SameLine(0.0f, 100.0f);

    ImGui::BeginGroup();
    if(ImGui::ImageButton("Open Project##1", open_project_png->getId(), buttonSize, {0, 1}, {1, 0})) {
        delete proj;
        std::string path = Utils::FileDialog::openFile();
        if(!path.empty()) {
            if(std::find(projectsPathCache.begin(), projectsPathCache.end(), path) == projectsPathCache.end()) {
                projectsPathCache.push_back(path);
            }
            proj = new Project(path);
            switchPage = true;
        }
    }
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (buttonSize.x + ImGui::GetStyle().FramePadding.x * 2) / 2 - ImGui::CalcTextSize("Open Project").x / 2);
    ImGui::Text("Open Project");
    ImGui::EndGroup();

    ImGui::PopStyleColor();
    ImGui::EndGroup();

    newProject->render();

    ImGui::End();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    if(switchPage) {
        static_cast<EditorApp*>(Cube::Engine::getApp())->switchPage(new EditorPage(proj));
    }
}
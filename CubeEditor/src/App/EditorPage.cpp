#include "EditorPage.h"

#include <string>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Cube/Core/Log.h"
#include "Cube/UI/FileDialog.h"
#include "Cube/Utils/Utils.h"
#include "Cube/Core/Engine.h"

#include "../Project/Project.h"
#include "../Views/EntityPropertyPanel.h"
#include "../Views/ResourcesPanel.h"
#include "../Views/ScenePanel.h"
#include "../Views/SceneView.h"
#include "../Views/AssetInspector.h"
#include "../Views/LogView.h"
#include "../Views/AnimationEditor.h"

using namespace Cube;

EditorPage::EditorPage(Project* project) : project(project) {
    views.push_back(std::make_unique<ScenePanel>(*this));
    views.push_back(std::make_unique<SceneView>(*this));
    views.push_back(std::make_unique<EntityPropertyPanel>(*this));
    views.push_back(std::make_unique<ResourcesPanel>(*this));
    views.push_back(std::make_unique<AssetInspector>(*this));
    views.push_back(std::make_unique<AnimationEditor>(*this));
    views.push_back(std::make_unique<LogView>(*this));

    auto& scenes = project->getScenes();
    if(!scenes.empty()) {
        selectedScene = &scenes.front();
    }

}

EditorPage::~EditorPage() {
    if(project) {
        project->getAssetExplorer().saveToFile(project->getConfig().projectDataDirectory + "/resources.cache", project->getConfig().assetPathMapFilePath);
    }
}

void EditorPage::render(float deltaTime) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()->Flags);
    // content
    ImGui::ShowDemoWindow();

    // MenuBar
    if(ImGui::BeginMainMenuBar()) {
        if(ImGui::BeginMenu("Project")) {
            if(ImGui::MenuItem("New Project")) {
            }
            if(ImGui::MenuItem("Open Project")) {
            }
            ImGui::EndMenu();
        }
        static bool showAddNewScene = false;
        if(ImGui::BeginMenu("Scene")) {
            if(ImGui::MenuItem("Add New Scene")) {
                showAddNewScene = true;
            }

            if(ImGui::MenuItem("Load Scene")) {
                std::string filePath = FileDialog::openFile("Scene File(.scene)\0*.scene\0" ,Engine::getApp()->getWindow()->getWin32Window());
                if(!filePath.empty()) {
                    Scene* scene = new Scene(filePath);
                    if(Utils::getFileName(filePath) == scene->getName()){
                        if(!project->hasScene(scene->getName())){
                            project->addScene(scene);
                            selectedScene = &project->getScenes().back();
                            if(!Utils::isFileInDirectory(filePath, project->getConfig().sceneDirectory)) {
                                Utils::copyFile(filePath, project->getConfig().sceneDirectory + "/" + scene->getName() + ".scene");
                            }
                        }else {
                            delete scene;
                            CB_WARN("The scene has existed"); // TODO: 提醒用户
                        }
                    }else {
                        delete scene;
                        CB_ERROR("The scene file name does not match the scene name"); // TODO: 提醒用户
                    }
                }
            }
            if(ImGui::MenuItem("Save Scene") && this->selectedScene) {
                if(!this->selectedScene->isSaved){
                    this->selectedScene->scene->serialize(project->getConfig().sceneDirectory + "/" + this->selectedScene->scene->getName() + ".scene");
                    this->selectedScene->isSaved = true;
                }
            }
            if(ImGui::MenuItem("Save All Scene")) {
                for(auto& scene : project->getScenes()){
                    if(!scene.isSaved){
                        scene.scene->serialize(project->getConfig().sceneDirectory + "/" + scene.scene->getName() + ".scene");
                        scene.isSaved = true;
                    }
                }
            }
            ImGui::EndMenu();
        }
        if(showAddNewScene) ImGui::OpenPopup("Add New Scene##1");
        if(ImGui::BeginPopupModal("Add New Scene##1")) {
            static char name[50] = {};
            ImGui::Text("Name: ");
            ImGui::SameLine();
            ImGui::InputText("##NameInputText", name, IM_ARRAYSIZE(name));

            static bool showTip = false;
            if(showTip) ImGui::Text("This scene has existed!");

            if(ImGui::Button("Add##3")) {
                if(!project->hasScene(name)){
                    project->addScene(new Scene(name, true));
                    selectedScene = &project->getScenes().back();
                    memset(name, '\0', sizeof(name));
                    showAddNewScene = false;
                    showTip = false;
                    ImGui::CloseCurrentPopup();
                }else {
                    showTip = true;
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel##3")) {
                memset(name, '\0', sizeof(name));
                showAddNewScene = false;
                showTip = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if(ImGui::BeginMenu("Resources")) {
            if(ImGui::MenuItem("Import Resources##1")) {
                importFromFileDialog();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    for(auto& view : views) {
        view->render(deltaTime);
    }

    thumbnailManager.tick();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void EditorPage::importFromFileDialog() {
    for(auto& path : FileDialog::openMultiFiles("Resources(.png.jpg)\0*.png;*.jpg\0All(.*)\0*.*\0", Engine::getApp()->getWindow()->getWin32Window())) {
        project->importResource(path);
    }
}

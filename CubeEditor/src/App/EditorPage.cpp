#include "EditorPage.h"

#include <filesystem>
#include <string>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Cube/Core/Log.h"
#include "Cube/Resource/NodeTree.h"
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
#include "../Utils/FileDialog.h"

EditorPage::EditorPage(Project* project) : project(project) {
    views.push_back(std::make_unique<ScenePanel>(*this));
    views.push_back(std::make_unique<SceneView>(*this));
    views.push_back(std::make_unique<EntityPropertyPanel>(*this));
    views.push_back(std::make_unique<ResourcesPanel>(*this));
    views.push_back(std::make_unique<AssetInspector>(*this));
    views.push_back(std::make_unique<AnimationEditor>(*this));
    views.push_back(std::make_unique<LogView>(*this));

    auto& docs = project->getDocuments();
    if(!docs.empty()) {
        selectedDoc = &docs.front();
    }

}

EditorPage::~EditorPage() {
    if(project) {
        project->getAssetExplorer().saveToFile(project->getConfig().projectDataDirectory / "resources.cache", project->getConfig().assetPathMapFilePath);
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
        static bool showAddNewDoc = false;
        if(ImGui::BeginMenu("Node Tree")) {
            if(ImGui::MenuItem("New Node Tree")) {
                showAddNewDoc = true;
            }

            if(ImGui::MenuItem("Load Node Tree")) {
                Cube::Path filePath = Utils::FileDialog::openFile("Load Node Tree", {{"Cube Node Tree (*.node)", "*.node"}}, project->getConfig().assetsDirectory);
                if(!filePath.empty()) {
                    Cube::NodeTree nodeTree(filePath);
                    std::unique_ptr<Cube::Node> root = nodeTree.instantiate();
                    if(!root) {
                        CB_ERROR("Failed to load node tree '{}'", filePath);
                    } else {
                        Cube::Path relPath = filePath.lexicallyRelative(project->getConfig().assetsDirectory);
                        if(relPath.empty()) {
                            relPath = Cube::Path(std::string(filePath.filename()));
                        }
                        const std::string identifier = "node:" + relPath.string();
                        if(!project->hasDocument(identifier)) {
                            // Keep the file under Assets and register it as a node resource.
                            Cube::Path target = project->getConfig().assetsDirectory / relPath;
                            if(!std::filesystem::equivalent(filePath.fspath(), target.fspath())) {
                                // TODO: 覆盖警告
                                std::filesystem::copy_file(filePath.fspath(), target.fspath(), std::filesystem::copy_options::overwrite_existing);
                            }
                            nlohmann::json importConfig;
                            importConfig["path"] = target.string();
                            if(project->getAssetExplorer().getAssetPathMap().contains(identifier)) {
                                project->getAssetExplorer().reimportResource(identifier, importConfig);
                            } else {
                                project->getAssetExplorer().createResource(identifier, importConfig);
                            }
                            project->addDocument(identifier, std::move(root));
                            selectedDoc = &project->getDocuments().back();
                        } else {
                            CB_WARN("The node tree has existed"); // TODO: 提醒用户
                        }
                    }
                }
            }
            if(ImGui::MenuItem("Save Node Tree") && this->selectedDoc) {
                if(!this->selectedDoc->isSaved){
                    const Cube::Path path = project->resolveResourcePath(this->selectedDoc->identifier);
                    Cube::NodeTree::save(path, *this->selectedDoc->root);
                    this->selectedDoc->isSaved = true;
                }
            }
            if(ImGui::MenuItem("Save All Node Trees")) {
                for(auto& doc : project->getDocuments()){
                    if(!doc.isSaved){
                        const Cube::Path path = project->resolveResourcePath(doc.identifier);
                        Cube::NodeTree::save(path, *doc.root);
                        doc.isSaved = true;
                    }
                }
            }
            ImGui::EndMenu();
        }
        if(showAddNewDoc) ImGui::OpenPopup("Add New Node Tree##1");
        if(ImGui::BeginPopupModal("Add New Node Tree##1")) {
            static char name[50] = {};
            ImGui::Text("Name: ");
            ImGui::SameLine();
            ImGui::InputText("##NameInputText", name, IM_ARRAYSIZE(name));

            static bool showTip = false;
            if(showTip) ImGui::Text("This node tree has existed!");

            if(ImGui::Button("Add##3")) {
                const std::string identifier = std::string("node:") + name + ".node";
                if(!project->hasDocument(identifier)){
                    auto root = std::make_unique<Cube::Node>(name);
                    const Cube::Path target = project->getConfig().assetsDirectory / (std::string(name) + ".node");
                    Cube::NodeTree::save(target, *root);
                    nlohmann::json importConfig;
                    importConfig["path"] = target.string();
                    if(project->getAssetExplorer().getAssetPathMap().contains(identifier)) {
                        project->getAssetExplorer().reimportResource(identifier, importConfig);
                    } else {
                        project->getAssetExplorer().createResource(identifier, importConfig);
                    }
                    project->addDocument(identifier, std::move(root));
                    selectedDoc = &project->getDocuments().back();
                    memset(name, '\0', sizeof(name));
                    showAddNewDoc = false;
                    showTip = false;
                    ImGui::CloseCurrentPopup();
                }else {
                    showTip = true;
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Cancel##3")) {
                memset(name, '\0', sizeof(name));
                showAddNewDoc = false;
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
    for(auto& path : Utils::FileDialog::openMultiFiles("Import Resources", {{"All Files (*.*)", "*.*"}, {"Texture", "*.png;*.jpg"}, {"AnimationClip", "*.anim"}}, project->getConfig().assetsDirectory)) {
        project->importResource(path);
    }
}

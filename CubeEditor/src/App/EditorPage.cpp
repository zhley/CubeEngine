#include "EditorPage.h"

#include <algorithm>
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
}

EditorPage::~EditorPage() {
    if(project) {
        project->getAssetExplorer().saveToFile(project->getConfig().projectDataDirectory / "resources.cache", project->getConfig().assetPathMapFilePath);
    }
}

NodeDocument* EditorPage::findDocument(const std::string& identifier) const {
    auto it = std::find_if(documents.begin(), documents.end(), [&identifier](const std::unique_ptr<NodeDocument>& doc) {
        return doc->getIdentifier() == identifier;
    });
    return it == documents.end() ? nullptr : it->get();
}

NodeDocument* EditorPage::openDocumentFromFile(const Cube::Path& filePath) {
    Cube::Path relPath = filePath.lexicallyRelative(project->getConfig().assetsDirectory);
    if(relPath.empty()) {
        relPath = Cube::Path(std::string(filePath.filename()));
    }
    const std::string identifier = "node:" + relPath.string();
    if(NodeDocument* existing = findDocument(identifier)) {
        setActiveDocument(existing);
        return existing;
    }

    std::unique_ptr<Cube::Node> root = project->loadNodeTree(filePath);
    if(!root) {
        return nullptr;
    }

    const Cube::Path target = project->getConfig().assetsDirectory / relPath;
    if(!std::filesystem::equivalent(filePath.fspath(), target.fspath())) {
        // TODO: 覆盖警告
        std::filesystem::copy_file(filePath.fspath(), target.fspath(), std::filesystem::copy_options::overwrite_existing);
    }
    nlohmann::json importConfig;
    importConfig["path"] = target.string();
    AssetExplorer& assets = project->getAssetExplorer();
    if(assets.getAssetPathMap().contains(identifier)) {
        assets.reimportResource(identifier, importConfig);
    } else {
        assets.createResource(identifier, importConfig);
    }

    documents.push_back(std::make_unique<NodeDocument>(identifier, std::move(root)));
    NodeDocument* doc = documents.back().get();
    doc->markSaved();
    setActiveDocument(doc);
    return doc;
}

NodeDocument* EditorPage::createAndOpenDocument(const std::string& name) {
    const std::string identifier = "node:" + name + ".node";
    if(findDocument(identifier) || std::filesystem::exists((project->getConfig().assetsDirectory / (name + ".node")).fspath())) {
        return nullptr;
    }
    if(!project->createNodeTreeFile(name)) {
        return nullptr;
    }
    auto root = std::make_unique<Cube::Node>(name);
    documents.push_back(std::make_unique<NodeDocument>(identifier, std::move(root)));
    NodeDocument* doc = documents.back().get();
    setActiveDocument(doc);
    return doc;
}

void EditorPage::saveDocument(NodeDocument* document) {
    if(!document || !document->getRoot()) {
        return;
    }
    if(project->saveNodeTree(document->getIdentifier(), *document->getRoot())) {
        document->markSaved();
    }
}

void EditorPage::saveAllDocuments() {
    for(auto& doc : documents) {
        if(doc->isDirty()) {
            saveDocument(doc.get());
        }
    }
}

void EditorPage::closeDocument(NodeDocument* document) {
    if(!document) {
        return;
    }
    const std::string identifier = document->getIdentifier();
    documents.erase(std::remove_if(documents.begin(), documents.end(), [&identifier](const std::unique_ptr<NodeDocument>& doc) {
        return doc->getIdentifier() == identifier;
    }), documents.end());
    if(activeDocument != document) {
        return;
    }
    activeDocument = nullptr;
    selectedNode = nullptr;
    if(!documents.empty()) {
        activeDocument = documents.front().get();
    }
}

void EditorPage::setActiveDocument(NodeDocument* document) {
    if(activeDocument != document) {
        activeDocument = document;
        selectedNode = nullptr;
    }
}

void EditorPage::setSelectedNode(Cube::Node* node) {
    selectedNode = node;
}

void EditorPage::clearNodeSelection() {
    selectedNode = nullptr;
}

void EditorPage::markActiveDirty() {
    if(activeDocument) {
        activeDocument->markDirty();
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
                    openDocumentFromFile(filePath);
                }
            }
            if(ImGui::MenuItem("Save Node Tree")) {
                saveDocument(activeDocument);
            }
            if(ImGui::MenuItem("Save All Node Trees")) {
                saveAllDocuments();
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
                if(createAndOpenDocument(name)){
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

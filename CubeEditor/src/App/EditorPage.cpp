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
#include "json.hpp"

#include "Project/Project.h"
#include "Views/EntityPropertyPanel.h"
#include "Views/ResourcesPanel.h"
#include "Views/HierarchyView.h"
#include "Views/SceneView.h"
#include "Views/ResourcesPanel.h"
#include "Views/AssetInspector.h"
#include "Views/LogView.h"
#include "Views/AnimationEditor.h"
#include "Utils/FileDialog.h"

NodeDocument* DocumentManager::find(const std::string& identifier) const {
    auto it = std::find_if(documents.begin(), documents.end(), [&identifier](const std::unique_ptr<NodeDocument>& doc) {
        return doc->getIdentifier() == identifier;
    });
    return it == documents.end() ? nullptr : it->get();
}

NodeDocument* DocumentManager::openFromFile(const Cube::Path& filePath) {
    const auto& map = project->getAssetExplorer().getAssetPathMap();
    std::string identifier;
    for(const auto& [id, cfg] : map) {
        if(!id.starts_with("node:")) {
            continue;
        }
        if(cfg.contains("path") && Cube::Path(cfg["path"].get<std::string>()) == filePath) {
            identifier = id;
            break;
        }
    }
    if(identifier.empty()) {
        identifier = project->importResource(filePath);
        if(identifier.empty()) {
            return nullptr;
        }
    }
    return open(identifier);
}

NodeDocument* DocumentManager::open(const std::string& identifier) {
    if(NodeDocument* existing = find(identifier)) {
        setActive(existing);
        return existing;
    }

    auto importer = project->getAssetExplorer().getAssetImporter(identifier);
    if(!importer) {
        CB_EDITOR_ERROR("DocumentManager::open: unknown resource '{}'", identifier);
        return nullptr;
    }
    const Cube::Path filePath(importer->get().at("path").get<std::string>());
    Cube::NodeTree nodeTree(filePath);
    std::unique_ptr<Cube::Node> root = nodeTree.instantiate();
    if(!root) {
        return nullptr;
    }

    documents.push_back(std::make_unique<NodeDocument>(identifier, std::move(root)));
    NodeDocument* doc = documents.back().get();
    doc->markSaved();
    setActive(doc);
    Cube::Engine::getApp()->getEventDispatcher().dispatch(ResourcesPanel::ResourceUsageEvent(identifier, true));
    return doc;
}

NodeDocument* DocumentManager::create() {
    Cube::Path filePath = Utils::FileDialog::saveFile(
        "New Node Tree",
        {{"Cube Node Tree (*.node)", "*.node"}},
        project->getConfig().assetsDirectory,
        ".node");
    if(filePath.empty()) {
        return nullptr;
    }
    auto root = std::make_unique<Cube::Node>(std::string(filePath.stem()));
    if(!Cube::NodeTree::save(filePath, *root)) {
        return nullptr;
    }
    const std::string identifier = project->importResource(filePath);
    if(identifier.empty()) {
        return nullptr;
    }
    documents.push_back(std::make_unique<NodeDocument>(identifier, std::move(root)));
    NodeDocument* doc = documents.back().get();
    doc->markSaved();
    setActive(doc);
    Cube::Engine::getApp()->getEventDispatcher().dispatch(ResourcesPanel::ResourceUsageEvent(identifier, true));
    return doc;
}

void DocumentManager::save(NodeDocument* document) {
    if(!document || !document->getRoot()) {
        return;
    }
    auto importer = project->getAssetExplorer().getAssetImporter(document->getIdentifier());
    if(!importer) {
        CB_EDITOR_ERROR("DocumentManager::save: unknown resource '{}'", document->getIdentifier());
        return;
    }
    if(Cube::NodeTree::save(Cube::Path(importer->get().at("path").get<std::string>()), *document->getRoot())) {
        document->markSaved();
    }
}

void DocumentManager::saveAll() {
    for(auto& doc : documents) {
        if(doc->isDirty()) {
            save(doc.get());
        }
    }
}

void DocumentManager::close(NodeDocument* document) {
    if(!document) {
        return;
    }
    const std::string identifier = document->getIdentifier();
    documents.erase(std::remove_if(documents.begin(), documents.end(), [document](const std::unique_ptr<NodeDocument>& doc) {
        return doc.get() == document;
    }), documents.end());
    Cube::Engine::getApp()->getEventDispatcher().dispatch(ResourcesPanel::ResourceUsageEvent(identifier, false));
    if(activeDocument != document) {
        return;
    }
    activeDocument = nullptr;
    if(!documents.empty()) {
        setActive(documents.front().get());
    }
}

EditorPage::EditorPage(Project* project) : project(project), documentManager(project) {
    views.push_back(std::make_unique<HierarchyView>(*this));
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
        if(ImGui::BeginMenu("Node Tree")) {
            if(ImGui::MenuItem("New Node Tree")) {
                documentManager.create();
            }

            if(ImGui::MenuItem("Load Node Tree")) {
                Cube::Path filePath = Utils::FileDialog::openFile("Load Node Tree", {{"Cube Node Tree (*.node)", "*.node"}}, project->getConfig().assetsDirectory);
                if(!filePath.empty()) {
                    documentManager.openFromFile(filePath);
                }
            }
            if(ImGui::MenuItem("Save Node Tree")) {
                documentManager.save(documentManager.getActive());
            }
            if(ImGui::MenuItem("Save All Node Trees")) {
                documentManager.saveAll();
            }
            ImGui::EndMenu();
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
    for(auto& path : Utils::FileDialog::openMultiFiles("Import Resources", {{"All Files (*.*)", "*.*"}, {"Texture", "*.png;*.jpg"}, {"AnimationClip", "*.anim"}, {"Script", "*.zt"}}, project->getConfig().assetsDirectory)) {
        project->importResources(path);
    }
}

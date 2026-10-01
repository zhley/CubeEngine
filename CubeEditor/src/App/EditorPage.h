#pragma once
#include <memory>
#include <string>
#include <vector>

#include "../Views/View.h"
#include "Page.h"
#include "Cube/Scene/Node.h"
#include "../Scene/EditorCamera.h"
#include "../Project/AssetExplorer.h"
#include "Views/ThumbnailManager.h"

class Project;
class EditorApp;

class NodeDocument {
public:
    // empty identifier means untitled, no file on disk yet.
    NodeDocument(const std::string& identifier, std::unique_ptr<Cube::Node> root) : identifier(identifier), root(std::move(root)) {};

    const std::string& getIdentifier() const { return identifier; }
    bool isUntitled() const { return identifier.empty(); }
    void setIdentifier(const std::string& id) { identifier = id; }
    Cube::Node* getRoot() const { return root.get(); }
    bool isDirty() const { return dirty; }
    void markDirty() { dirty = true; }
    void markSaved() { dirty = false; }

    Cube::Node* getSelectedNode() const { return selectedNode; }
    void selectNode(Cube::Node* node) { selectedNode = node; }

private:
    std::string identifier;
    std::unique_ptr<Cube::Node> root;
    bool dirty = false;
    Cube::Node* selectedNode = nullptr;
};

// Open documents are editor runtime state (not persisted).
class DocumentManager {
public:
    DocumentManager() = default;
    explicit DocumentManager(Project* project) : project(project) {}

    const std::vector<std::unique_ptr<NodeDocument>>& getDocuments() const { return documents; }
    NodeDocument* getActive() const { return activeDocument; }
    void setActive(NodeDocument* document) { activeDocument = document; }

    NodeDocument* find(const std::string& identifier) const;
    NodeDocument* openFromFile(const Cube::Path& filePath);
    NodeDocument* open(const std::string& identifier);
    NodeDocument* createUntitled();
    void save(NodeDocument* document);
    void saveAll();
    void close(NodeDocument* document);

private:
    Project* project = nullptr;
    std::vector<std::unique_ptr<NodeDocument>> documents;
    NodeDocument* activeDocument = nullptr;
};

class EditorPage : public Page {
public:
    EditorPage(Project* project);
    ~EditorPage() override;

    void render(float deltaTime) override;
    Page::Type getType() const override { return Page::Type::Editor; }

    void importFromFileDialog();

    Project* getProject() const { return project.get(); }

    AssetNode* selectedAssetNode = nullptr;
    EditorCamera editorCamera;

    DocumentManager documentManager;

    ThumbnailManager thumbnailManager;

private:
    std::vector<std::unique_ptr<View>> views;
    std::unique_ptr<Project> project;
};

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

class NodeDocument;
class Project;
class EditorApp;

class EditorPage : public Page {
public:
    EditorPage(Project* project);
    ~EditorPage() override;

    void render(float deltaTime) override;
    Page::Type getType() const override { return Page::Type::Editor; }

    void importFromFileDialog();

    Project* getProject() const { return project.get(); }

    // Open documents are editor runtime state (not persisted).
    const std::vector<std::unique_ptr<NodeDocument>>& getDocuments() const { return documents; }
    NodeDocument* findDocument(const std::string& identifier) const;
    NodeDocument* openDocumentFromFile(const Cube::Path& filePath);
    NodeDocument* createAndOpenDocument(const std::string& name);
    void saveDocument(NodeDocument* document);
    void saveAllDocuments();
    void closeDocument(NodeDocument* document);

    void setActiveDocument(NodeDocument* document);
    void setSelectedNode(Cube::Node* node);
    void clearNodeSelection();
    void markActiveDirty();

    // Active document pointer is stable (documents are unique_ptr).
    // selectedNode is only valid inside that document tree.
    NodeDocument* activeDocument = nullptr;
    Cube::Node* selectedNode = nullptr;

    AssetNode* selectedAssetNode = nullptr;
    EditorCamera editorCamera;

    ThumbnailManager thumbnailManager;

private:
    std::vector<std::unique_ptr<View>> views;
    std::unique_ptr<Project> project;
    std::vector<std::unique_ptr<NodeDocument>> documents;
};

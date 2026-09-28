#pragma once
#include <memory>
#include <vector>

#include "../Views/View.h"
#include "Page.h"
#include "Cube/Scene/Node.h"
#include "../Scene/EditorCamera.h"
#include "../Project/AssetExplorer.h"
#include "Views/ThumbnailManager.h"

struct NodeDocument;
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

    // Editor state
    NodeDocument* selectedDoc = nullptr;
    Cube::Node* selectedNode = nullptr;
    AssetNode* selectedAssetNode = nullptr;
    EditorCamera editorCamera;

    ThumbnailManager thumbnailManager;

private:
    std::vector<std::unique_ptr<View>> views;
    std::unique_ptr<Project> project;
};

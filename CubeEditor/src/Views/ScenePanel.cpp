#include "ScenePanel.h"

#include "../App/EditorPage.h"
#include "../Project/Project.h"
#include "../Utils/ImGuiExternal.h"
#include "Cube/Core/Log.h"
#include "Cube/Scene/Entity.h"
#include "Cube/Scene/Scene.h"

#include <imgui/imgui.h>

void ScenePanel::render(float deltaTime) {
    ImGui::Begin("Scene Panel");

    Project* project = editorPage.getProject();
    if(!project) {
        ImGui::End();
        return;
    }

    static char name[50] = {};
    static SceneData* addScene = nullptr;
    static Cube::Entity* addParent = nullptr;
    static std::unique_ptr<ModalPopup> addEntityPopup = std::make_unique<ModalPopup>("Add Entity", [] {
        ImGui::Text("Name:");
        ImGui::InputText("##input", name, IM_ARRAYSIZE(name));
    }, [this] {
        if(!addScene) {
            addEntityPopup->close();
            return;
        }
        if(addParent) {
            addParent->addChild(name);
        } else {
            addScene->scene->createEntity(name);
        }
        addScene->isSaved = false;
        addEntityPopup->close();
    }, [] {
        memset(name, '\0', sizeof(name));
        addScene = nullptr;
        addParent = nullptr;
    });
    addEntityPopup->render();

    auto drawEntityTree = [&](auto&& self, SceneData& sceneData, Cube::Entity* entity) -> void {
        ImGui::PushID(entity);
        const bool isSelected = editorPage.selectedEntity == entity;
        const bool hasChildren = !entity->getChildren().empty();
        ImGuiTreeNodeFlags flags = Utils::TREENODE_FLAGS;
        if(isSelected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if(!hasChildren) {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        bool isOpen = ImGui::TreeNodeEx("##entity", flags, "%s", entity->getName().c_str());
        if(ImGui::IsItemClicked()) {
            editorPage.selectedScene = &sceneData;
            editorPage.selectedEntity = entity;
        }

        if(ImGui::BeginPopupContextItem()) {
            if(ImGui::MenuItem("Add Child")) {
                addScene = &sceneData;
                addParent = entity;
                addEntityPopup->open();
            }
            if(entity->getParent() && ImGui::MenuItem("Delete")) {
                if(entity == editorPage.selectedEntity) {
                    editorPage.selectedEntity = nullptr;
                }
                entity->getParent()->removeChild(entity);
                sceneData.isSaved = false;
            }
            ImGui::EndPopup();
        }

        if(hasChildren && isOpen) {
            for(const auto& child : entity->getChildren()) {
                self(self, sceneData, child.get());
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    };

    for(SceneData& scene : project->getScenes()) {
        ImGui::PushID(scene.scene);
        const bool isSceneSelected = editorPage.selectedScene == &scene;
        ImGuiTreeNodeFlags sceneFlags = Utils::TREENODE_FLAGS;
        if(isSceneSelected) {
            sceneFlags |= ImGuiTreeNodeFlags_Selected;
        }

        std::string sceneLabel = scene.scene->getName() + (scene.isSaved ? "" : "*");
        bool sceneOpen = ImGui::TreeNodeEx("##scene", sceneFlags, "%s", sceneLabel.c_str());
        if(ImGui::IsItemClicked()) {
            editorPage.selectedScene = &scene;
            editorPage.selectedEntity = nullptr;
        }

        if(ImGui::BeginPopupContextItem()) {
            if(ImGui::MenuItem("Add Entity")) {
                addScene = &scene;
                addParent = nullptr;
                addEntityPopup->open();
            }
            ImGui::EndPopup();
        }

        if(sceneOpen) {
            Cube::Entity* root = scene.scene->getRootEntity();
            for(const auto& child : root->getChildren()) {
                drawEntityTree(drawEntityTree, scene, child.get());
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    ImGui::End();
}

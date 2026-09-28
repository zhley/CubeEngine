#include "ScenePanel.h"

#include "../App/EditorPage.h"
#include "../Project/Project.h"
#include "../Utils/ImGuiExternal.h"
#include "Cube/Core/Log.h"
#include "Cube/Scene/Node.h"

#include <imgui/imgui.h>

void ScenePanel::render(float deltaTime) {
    ImGui::Begin("Hierarchy");

    Project* project = editorPage.getProject();
    if(!project) {
        ImGui::End();
        return;
    }

    NodeDocument* doc = editorPage.selectedDoc;
    if(!doc || !doc->root) {
        ImGui::TextUnformatted("No open node tree.");
        ImGui::End();
        return;
    }

    static char name[50] = {};
    static Cube::Node* addParent = nullptr;
    static std::unique_ptr<ModalPopup> addNodePopup = std::make_unique<ModalPopup>("Add Node", [] {
        ImGui::Text("Name:");
        ImGui::InputText("##input", name, IM_ARRAYSIZE(name));
    }, [this] {
        if(!editorPage.selectedDoc || !editorPage.selectedDoc->root) {
            addNodePopup->close();
            return;
        }
        if(addParent) {
            addParent->addChild(name);
        } else {
            editorPage.selectedDoc->root->addChild(name);
        }
        editorPage.selectedDoc->isSaved = false;
        addNodePopup->close();
    }, [] {
        memset(name, '\0', sizeof(name));
        addParent = nullptr;
    });
    addNodePopup->render();

    auto drawNodeTree = [&](auto&& self, Cube::Node* node) -> void {
        ImGui::PushID(node);
        const bool isSelected = editorPage.selectedNode == node;
        const bool hasChildren = !node->getChildren().empty();
        ImGuiTreeNodeFlags flags = Utils::TREENODE_FLAGS;
        if(isSelected) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        if(!hasChildren) {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        bool isOpen = ImGui::TreeNodeEx("##node", flags, "%s", node->getName().c_str());
        if(ImGui::IsItemClicked()) {
            editorPage.selectedNode = node;
        }

        if(ImGui::BeginPopupContextItem()) {
            if(ImGui::MenuItem("Add Child")) {
                addParent = node;
                addNodePopup->open();
            }
            if(node->getParent() && ImGui::MenuItem("Delete")) {
                if(node == editorPage.selectedNode) {
                    editorPage.selectedNode = nullptr;
                }
                node->getParent()->removeChild(node);
                doc->isSaved = false;
            }
            ImGui::EndPopup();
        }

        if(hasChildren && isOpen) {
            for(Cube::Node* child : node->getChildren()) {
                self(self, child);
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    };

    if(ImGui::BeginPopupContextWindow("HierarchyEmpty", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
        if(ImGui::MenuItem("Add Node")) {
            addParent = doc->root.get();
            addNodePopup->open();
        }
        ImGui::EndPopup();
    }

    drawNodeTree(drawNodeTree, doc->root.get());

    ImGui::End();
}

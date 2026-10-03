#include "HierarchyView.h"

#include "../App/EditorPage.h"
#include "../Project/Project.h"
#include "../Utils/ImGuiExternal.h"
#include "Cube/Core/Log.h"
#include "Cube/Scene/Node.h"
#include "Scene/EditorNodeAccess.h"

#include <imgui/imgui.h>

void HierarchyView::render(float deltaTime) {
    ImGui::Begin("Hierarchy");

    Project* project = editorPage.getProject();
    if(!project) {
        ImGui::End();
        return;
    }

    NodeDocument* doc = editorPage.documentManager.getActive();
    if(!doc || !doc->getRoot()) {
        ImGui::TextUnformatted("No open node tree.");
        ImGui::End();
        return;
    }

    static char name[50] = {};
    static Cube::Node* addParent = nullptr;
    static bool addNodePopupOpen = false;

    if(addNodePopupOpen) {
        ImGui::OpenPopup("Add Node");
    }
    if(ImGui::BeginPopupModalSuper("Add Node", &addNodePopupOpen, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Name:");
        ImGui::InputText("##input", name, IM_ARRAYSIZE(name));

        constexpr float buttonWidth = 100.0f;
        constexpr float spacing = 100.0f;
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - (buttonWidth * 2 + ImGui::GetStyle().FramePadding.x * 2 + spacing) / 2);
        ImGui::BeginGroup();
        if(ImGui::Button("OK", ImVec2(buttonWidth, 0)) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            NodeDocument* doc = editorPage.documentManager.getActive();
            if(doc && doc->getRoot()) {
                if(addParent) {
                    EditorNodeAccess::addChild(*addParent, name);
                } else {
                    EditorNodeAccess::addChild(*doc->getRoot(), name);
                }
                doc->markDirty();
            }
            addNodePopupOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(0.0f, spacing);
        if(ImGui::Button("Cancel", ImVec2(buttonWidth, 0))) {
            addNodePopupOpen = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndGroup();

        ImGui::EndPopup();
    } else if(!addNodePopupOpen) {
        memset(name, '\0', sizeof(name));
        addParent = nullptr;
    }

    Cube::Node* pendingDeleteNode = nullptr;

    auto drawNodeTree = [&](auto&& self, Cube::Node* node) -> void {
        ImGui::PushID(node);
        const bool isSelected = editorPage.documentManager.getActive()->getSelectedNode() == node;
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
            editorPage.documentManager.getActive()->selectNode(node);
        }

        if(ImGui::BeginPopupContextItem()) {
            if(ImGui::MenuItem("Add Child")) {
                addParent = node;
                addNodePopupOpen = true;
            }
            if(node->getParent() && ImGui::MenuItem("Delete")) {
                if(node == editorPage.documentManager.getActive()->getSelectedNode()) {
                    editorPage.documentManager.getActive()->selectNode(nullptr);
                }
                pendingDeleteNode = node;
                doc->markDirty();
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
            addParent = doc->getRoot();
            addNodePopupOpen = true;
        }
        ImGui::EndPopup();
    }

    drawNodeTree(drawNodeTree, doc->getRoot());

    if(pendingDeleteNode) {
        if(Cube::Node* parent = pendingDeleteNode->getParent()) {
            EditorNodeAccess::removeChild(*parent, pendingDeleteNode);
        }
        pendingDeleteNode = nullptr;
    }

    ImGui::End();
}

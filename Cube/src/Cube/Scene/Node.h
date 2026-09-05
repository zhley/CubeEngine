#pragma once

#include <memory>
#include <vector>

#include "json.hpp"
#include "glm/glm.hpp"

#include "Component.h"
#include "Transform.h"
#include "Cube/Core/Log.h"
#include "Cube/Reflection/ClassRegistry.h"
#include "Cube/Reflection/Type.h"

namespace Cube {

    class NodeTree;

    // The game is a single rooted tree of nodes. A node only carries basic state
    // (name, hierarchy and lifetime); every other property and behaviour is
    // provided by the components mounted on it. Each node implicitly carries a
    // Transform component as its basic attribute.
    //
    // There is no explicit "scene": switching or stacking scenes is done by
    // detaching subtrees from a parent and attaching instantiated node trees.
    class Node {
    public:
        Node() = default;
        explicit Node(const std::string& name) : name(name) {}
        ~Node() = default;

        void update(float delta);

        // ---- component mounting ----
        // One component per type at most. Mounting is applied at the end of the
        // current update, then start() is invoked on the component.
        Component* addComponent(std::unique_ptr<Component> component);

        template<typename T>
        T* addComponent() {
            static_assert(std::is_base_of_v<Component, T>);
            return static_cast<T*>(addComponent(std::make_unique<T>()));
        }

        Component* addComponent(const std::string& typeName);

        void removeComponent(TypeID typeID);

        template<typename T>
        void removeComponent() {
            static_assert(std::is_base_of_v<Component, T>);
            removeComponent(getTypeID<T>());
        }

        void removeComponent(const std::string& typeName);

        template<typename T>
        T* getComponent() const {
            static_assert(std::is_base_of_v<Component, T>);
            TypeID typeID = getTypeID<T>();
            auto it = componentsMap.find(typeID);
            if(it == componentsMap.end()) {
                CB_CORE_ERROR("Node::getComponent<T>(): component of type '{}' does not exist", ClassRegistry::get().getClass<T>()->getName());
                return nullptr;
            }
            return static_cast<T*>(it->second);
        }

        Component* getComponent(const std::string& typeName) const;

        const std::vector<std::unique_ptr<Component>>& getComponents() const { return components; }

        template<typename T>
        bool hasComponent() const {
            static_assert(std::is_base_of_v<Component, T>);
            TypeID typeID = getTypeID<T>();
            return componentsMap.find(typeID) != componentsMap.end();
        }

        bool hasComponent(const std::string& typeName) const;

        // ---- lifetime ----
        bool isAlive() const { return alive; }
        // Marks this node (and therefore its whole subtree) destroyed. The node
        // is removed from its parent at the end of the next update.
        void destroy() { alive = false; }

        // ---- basic attributes ----
        const std::string& getName() const { return name; }
        void setName(const std::string& name) { this->name = name; }
        // The implicitly mounted Transform of this node.
        Transform& getTransform() { return transform; }
        const Transform& getTransform() const { return transform; }

        // ---- tree structure ----
        Node* getParent() const { return parent; }
        const std::vector<std::unique_ptr<Node>>& getChildren() const { return children; }

        // Creates a new empty child node.
        Node* addChild(const std::string& name);
        // Attaches an existing subtree (e.g. an instantiated .node blueprint).
        // Used to switch or stack scenes below this node.
        Node* addChild(std::unique_ptr<Node> child);
        // Marks a direct child (and its subtree) destroyed.
        void removeChild(Node* child);
        void removeChild(const std::string& name);
        // Depth-first search for the first descendant with the given name.
        Node* findChild(const std::string& name) const;

        // ---- subtree queries ----
        // All nodes of this subtree, this node included, depth-first.
        std::vector<Node*> getAllNodes() const;

        template<typename... Types>
        std::vector<Node*> getNodesWith() const {
            static_assert((std::is_base_of_v<Component, Types> && ...));
            std::vector<Node*> result;
            for(Node* node : getAllNodes()) {
                bool hasAll = true;
                ((hasAll = hasAll && node->hasComponent<Types>()), ...);
                if(hasAll) {
                    result.push_back(node);
                }
            }
            return result;
        }

        Node* addChildFromTree(NodeTree* nodeTree);

        // ---- serialization ----
        void deserialize(const nlohmann::json& data);
        nlohmann::json serialize() const;

    private:
        std::string name;  // Temporarily used as the unique identifier
        bool alive = true;

        // Implicitly mounted Transform, not part of components/componentsMap.
        Transform transform = Transform(this);

        std::vector<std::unique_ptr<Component>> components;  // for iteration
        std::unordered_map<TypeID, Component*> componentsMap;  // for lookup

        Node* parent = nullptr;
        std::vector<std::unique_ptr<Node>> children;

        std::vector<std::unique_ptr<Component>> pendingAdd;
        std::vector<TypeID> pendingDestroy;
        std::vector<char> addOrDestroy; // 0 for add, 1 for destroy  denote the order of add and destroy
        std::vector<Component*> pendingStart;

        void processStart();
        void processAddAndDestroy();
        void processChildDestroy();
    };

}

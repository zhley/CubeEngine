#pragma once

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "json.hpp"
#include "glm/glm.hpp"

#include "Component.h"
#include "Cube/Reflection/Type.h"

namespace Cube {

class NodeTree;

// The game is a single rooted tree of nodes. A node only carries basic state
// (name, hierarchy and lifetime); every other property and behaviour is
// provided by the components mounted on it. 
//
// There is no explicit "scene": switching or stacking scenes is done by
// detaching subtrees from a parent and attaching instantiated node trees.
class Node {
public:
    // transform
    glm::vec2 pos = {0.0f, 0.0f};
    float rotation = 0.0f; // degrees
    glm::vec2 scale = {1.0f, 1.0f};

    explicit Node(const std::string& name) : name(name) {}
    ~Node() = default;

    void update(float delta);

    glm::mat4 getLocalMatrix() const;
    glm::mat4 getWorldMatrix() const;

    Component* addComponent(std::unique_ptr<Component> component);
    Component* addComponent(const std::string& typeName);

    template<typename T>
    requires std::is_base_of_v<Component, T>
    T* addComponent() {
        return static_cast<T*>(addComponent(std::make_unique<T>()));
    }

    void removeComponent(TypeID typeID);
    void removeComponent(const std::string& typeName);

    template<typename T>
    requires std::is_base_of_v<Component, T>
    void removeComponent() {
        removeComponent(getTypeID<T>());
    }

    Component* getComponent(TypeID typeID) const;
    Component* getComponent(const std::string& typeName) const;

    template<typename T>
    requires std::is_base_of_v<Component, T>
    T* getComponent() const {
        return static_cast<T*>(getComponent(getTypeID<T>()));
    }

    bool hasComponent(TypeID typeID) const;
    bool hasComponent(const std::string& typeName) const;
    
    template<typename T>
    requires std::is_base_of_v<Component, T>
    bool hasComponent() const {
        return hasComponent(getTypeID<T>());
    }
    
    const std::vector<Component*>& getComponents() const { return componentsCache; }

    const std::string& getName() const { return name; }

    Node* getParent() const { return parent; }
    const std::vector<Node*>& getChildren() const { return childrenCache; }

    Node* addChild(const std::string& name);
    Node* addChild(std::unique_ptr<Node> child);
    Node* addChild(NodeTree* nodeTree);
    void removeChild(Node* child);
    void removeChild(const std::string& name);
    Node* findChild(const std::string& name) const;

    // depth-first
    // F: return false to stop iteration
    // if iteration is stopped, return false
    template<typename F>
    requires requires(F f, Node* p) { {f(p)} -> std::same_as<bool>; }
    bool forEachNode(F&& f) {
        if (!f(this)) return false;
        for(Node* child : childrenCache) {
            if (!child->forEachNode(f)) return false;
        }
        return true;
    }

    void deserialize(const nlohmann::json& data);
    nlohmann::json serialize() const;

private:
    std::string name;  // Temporarily used as the unique identifier
    Node* parent = nullptr;
    
    std::unordered_map<TypeID, std::unique_ptr<Component>> components; // owner
    std::vector<Component*> componentsCache; // cache for iteration
    std::unordered_map<std::string, std::unique_ptr<Node>> children;
    std::vector<Node*> childrenCache;

    struct Command {
        enum class Type {
            ChildAdd,
            ChildRemove,
            ComponentAdd,
            ComponentRemove,
        };
        Type type;
        std::variant<std::unique_ptr<Node>, std::string, std::unique_ptr<Component>, TypeID> data;
    };
    std::vector<Command> commandQueue;
    std::vector<Component*> pendingStart;

    void flushCommands();
};

}

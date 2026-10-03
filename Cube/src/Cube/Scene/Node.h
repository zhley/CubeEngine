#pragma once

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

#include "json.hpp"
#include "glm/glm.hpp"

#include "Component.h"
#include "Cube/Script/ScriptComponent.h"
#include "Cube/Reflection/Type.h"

// Editor-only direct scene manipulation, implemented in CubeEditor. It is granted
// access through friendship so it is never part of the game build.
class EditorNodeAccess;

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

    Node() = default;
    explicit Node(const std::string& name) : name(name) {}
    ~Node() = default;

    void update(float delta);

    glm::mat4 getLocalMatrix() const;
    glm::mat4 getWorldMatrix() const;

    Component* addComponent(std::unique_ptr<Component> component);
    Component* addComponent(const std::string& typeName);

    template<typename T>
    requires (std::is_base_of_v<Component, T> && !std::is_same_v<T, ScriptComponent>)
    T* addComponent() {
        return static_cast<T*>(addComponent(std::make_unique<T>()));
    }

    template<typename T>
    requires std::is_same_v<T, ScriptComponent>
    ScriptComponent* addComponent(const std::string& scriptIdentifier, const std::string& scriptComponentName) {
        return static_cast<ScriptComponent*>(addComponent(std::make_unique<T>(this, scriptIdentifier, scriptComponentName)));
    }

    void removeComponent(TypeID typeID);
    void removeComponent(const std::string& typeName);

    template<typename T>
    requires (std::is_base_of_v<Component, T> && !std::is_same_v<T, ScriptComponent>)
    void removeComponent() {
        removeComponent(getTypeID<T>());
    }

    template<typename T>
    requires std::is_same_v<T, ScriptComponent>
    void removeComponent(const std::string& scriptComponentName) {
        removeComponent(scriptComponentName);
    }

    Component* getComponent(TypeID typeID) const;
    Component* getComponent(const std::string& typeName) const;

    template<typename T>
    requires (std::is_base_of_v<Component, T> && !std::is_same_v<T, ScriptComponent>)
    T* getComponent() const {
        return static_cast<T*>(getComponent(getTypeID<T>()));
    }

    template<typename T>
    requires std::is_same_v<T, ScriptComponent>
    T* getComponent(const std::string& scriptComponentName) const {
        return static_cast<T*>(getComponent(scriptComponentName));
    }

    bool hasComponent(TypeID typeID) const;
    bool hasComponent(const std::string& typeName) const;
    
    template<typename T>
    requires std::is_base_of_v<Component, T>
    bool hasComponent() const {
        return hasComponent(getTypeID<T>());
    }

    template<typename T>
    requires std::is_same_v<T, ScriptComponent>
    bool hasComponent(const std::string& scriptComponentName) const {
        return hasComponent(scriptComponentName);
    }
    
    const std::vector<Component*>& getComponents() const { return componentsCache; }

    const std::string& getName() const { return name; }

    Node* getParent() const { return parent; }
    const std::vector<Node*>& getChildren() const { return childrenCache; }

    Node* addChild(const std::string& name);
    Node* addChild(std::unique_ptr<Node> child);
    Node* addChildFrom(const ResPtr<NodeTree>& nodeTree);
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
    friend class ::EditorNodeAccess;

    std::string name;  // Temporarily used as the unique identifier
    Node* parent = nullptr;
    
    std::unordered_map<TypeID, std::unique_ptr<Component>> components; // owner
    std::unordered_map<std::string, std::unique_ptr<ScriptComponent>> scriptComps; // a node can keep multiple script components.
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
        std::variant<std::unique_ptr<Node>, std::string, std::unique_ptr<Component>, std::variant<TypeID, std::string>> data;
    };
    std::vector<Command> commandQueue;
    std::vector<Component*> pendingStart;

    void flushCommands();
};

}

#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <utility>

#include "Cube/Scene/Node.h"

// Editor-only access to a Node's internals.
//
// Node defers structural changes through a command queue that is flushed during
// update(); that exists so the runtime can mutate the hierarchy/components while it
// is iterating the caches. The editor never mutates from within such an iteration,
// so it can apply changes directly instead of relying on the queue.
//
// This type is compiled into CubeEditor only and is granted access via friendship
// (see Node/Component), so it is never part of the game build.
class EditorNodeAccess {
public:
    static Cube::Node* addChild(Cube::Node& parent, std::unique_ptr<Cube::Node> child);
    static Cube::Node* addChild(Cube::Node& parent, const std::string& name);
    static void removeChild(Cube::Node& parent, Cube::Node* child);
    static void removeChild(Cube::Node& parent, const std::string& name);

    static Cube::Component* addComponent(Cube::Node& node, std::unique_ptr<Cube::Component> component);

    template<typename T, typename... Args>
    requires (std::is_base_of_v<Cube::Component, T> && !std::is_same_v<T, Cube::ScriptComponent>)
    static T* addComponent(Cube::Node& node, Args&&... args) {
        return static_cast<T*>(addComponent(node, std::make_unique<T>(std::forward<Args>(args)...)));
    }

    static void removeComponent(Cube::Node& node, Cube::TypeID typeID);
    static void removeComponent(Cube::Node& node, const std::string& typeName);
};

#include "Node.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "glm/ext/matrix_transform.hpp"

#include "Cube/Core/Log.h"
#include "Cube/Reflection/Serializer.h"
#include "Cube/Resource/NodeTree.h"

namespace Cube {

void Node::update(float delta) {
    flushCommands(); 

    int p = 0;
    while (p < pendingStart.size()) {
        pendingStart[p]->start();
        p++;
    }
    pendingStart.clear();
    flushCommands(); 

    for (auto& component : componentsCache) {
        component->update(delta);
    }

    for (auto& child : childrenCache) {
        child->update(delta);
    }

    flushCommands();
}

glm::mat4 Node::getLocalMatrix() const {
    glm::mat4 localMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
    localMatrix = glm::rotate(localMatrix, glm::radians(rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    localMatrix = glm::scale(localMatrix, glm::vec3(scale, 1.0f));
    return localMatrix;
}

glm::mat4 Node::getWorldMatrix() const {
    glm::mat4 worldMatrix = getLocalMatrix();
    if(parent) {
        worldMatrix = parent->getWorldMatrix() * worldMatrix;
    }
    return worldMatrix;
}

Component* Node::addComponent(std::unique_ptr<Component> component) {
    if (!component) {
        CB_CORE_ERROR("Node::addComponent(): component is nullptr");
        return nullptr;
    }
    Component* result = component.get();
    result->node = this;
    pendingStart.push_back(result);
    commandQueue.push_back({Command::Type::ComponentAdd, std::move(component)});
    return result;
}

Component* Node::addComponent(const std::string& typeName) {
    Class* classInfo = ClassRegistry::get().getClass(typeName);
    if (!classInfo) {
        CB_CORE_ERROR("Node::addComponent(): Unknown component type '{}'", typeName);
        return nullptr;
    }
    Any instance = classInfo->createInstance();
    Component* component = instance.moveToBase<Component>();
    if (!component) {
        CB_CORE_ERROR("Node::addComponent(): Type '{}' does not derive from Component", typeName);
        return nullptr;
    }
    return addComponent(std::unique_ptr<Component>(component));
}

void Node::removeComponent(TypeID typeID) {
    commandQueue.push_back({Command::Type::ComponentRemove, typeID});
}

void Node::removeComponent(const std::string& typeName) {
    Class* classInfo = ClassRegistry::get().getClass(typeName);
    if (!classInfo) {
        CB_CORE_ERROR("Node::removeComponent(): Unknown component type '{}'", typeName);
        return;
    }
    removeComponent(classInfo->getTypeID());
}

Component* Node::getComponent(TypeID typeID) const {
    auto it = components.find(typeID);
    return it == components.end() ? nullptr : it->second.get();
}

Component* Node::getComponent(const std::string& typeName) const {
    Class* classInfo = ClassRegistry::get().getClass(typeName);
    if (!classInfo) {
        CB_CORE_ERROR("Node::getComponent(): Unknown component type '{}'", typeName);
        return nullptr;
    }
    return getComponent(classInfo->getTypeID());
}

bool Node::hasComponent(TypeID typeID) const {
    return components.contains(typeID);
}

bool Node::hasComponent(const std::string& typeName) const {
    Class* classInfo = ClassRegistry::get().getClass(typeName);
    return classInfo && hasComponent(classInfo->getTypeID());
}

Node* Node::addChild(const std::string& name) {
    return addChild(std::make_unique<Node>(name));
}

Node* Node::addChild(std::unique_ptr<Node> child) {
    if (!child) {
        CB_CORE_ERROR("Node::addChild(): child is nullptr");
        return nullptr;
    }
    if (child.get() == this) {
        CB_CORE_ERROR("Node::addChild(): Node '{}' cannot be a child of itself", name);
        return nullptr;
    }
    Node* result = child.get();
    result->parent = this;
    commandQueue.push_back({Command::Type::ChildAdd, std::move(child)});
    return result;
}

Node* Node::addChild(NodeTree* nodeTree) {
    std::unique_ptr<Node> subtree = nodeTree->instantiate();
    if (!subtree) {
        CB_CORE_ERROR("Node::addChild(): Failed to instantiate the node tree");
        return nullptr;
    }
    return addChild(std::move(subtree));
}

void Node::removeChild(Node* child) {
    if (!child) {
        CB_CORE_ERROR("Node::removeChild(): child is nullptr");
        return;
    }
    removeChild(child->name);
}

void Node::removeChild(const std::string& childName) {
    commandQueue.push_back({Command::Type::ChildRemove, childName});
}

Node* Node::findChild(const std::string& childName) const {
    auto it = children.find(childName);
    return it == children.end() ? nullptr : it->second.get();
}

void Node::deserialize(const nlohmann::json& data) {
    try {
        name = data["name"];
        pos = {data["pos"][0], data["pos"][1]};
        rotation = data["rotation"];
        scale = {data["scale"][0], data["scale"][1]};
        for(const nlohmann::json& c : data["components"]) {
            Class* classInfo = ClassRegistry::get().getClass(c["type"].get<std::string>());
            Any componentIns = Serializer::get().deserialize(classInfo->getTypeID(), c);
            Component* component = componentIns.moveToBase<Component>();
            addComponent(std::unique_ptr<Component>(component));
        }
        for(const nlohmann::json& childData : data["children"]) {
            // the child is built before being queued, so it never has to be observed through a raw pointer
            std::unique_ptr<Node> child = std::make_unique<Node>(childData.value("name", std::string()));
            child->deserialize(childData);
            addChild(std::move(child));
        }
    } catch (const std::exception& e) {
        CB_CORE_ERROR("Node::deserialize(): Failed to deserialize node '{}': {}", name, e.what());
    }
    flushCommands();
}

nlohmann::json Node::serialize() const {
    nlohmann::json data;
    data["name"] = name;
    nlohmann::json tr;
    tr["pos"] = {pos.x, pos.y};
    tr["rotation"] = rotation;
    tr["scale"] = {scale.x, scale.y};
    data["transform"] = tr;
    data["components"] = nlohmann::json::array();
    for(const auto& [typeID, component] : components) {
        Class* classInfo = ClassRegistry::get().getClass(typeID);
        nlohmann::json c = Serializer::get().serialize(typeID, Any(component.get()));
        c["type"] = classInfo->getName();
        data["components"].push_back(c);
    }
    data["children"] = nlohmann::json::array();
    for(Node* child : childrenCache) {
        data["children"].push_back(child->serialize());
    }
    return data;
}

void Node::flushCommands() {
    if (commandQueue.empty()) return;
    int childEnd = childrenCache.size();
    int componentEnd = componentsCache.size();
    for (Command& cmd : commandQueue) {
        switch (cmd.type) {
            case Command::Type::ChildAdd: {
                auto child = std::move(std::get<std::unique_ptr<Node>>(cmd.data));
                auto it = children.find(child->name);
                if (it != children.end()) break; // child with the same name already exists, ignore
                if (childEnd < childrenCache.size()) {
                    childrenCache[childEnd++] = child.get();
                } else {
                    childrenCache.push_back(child.get());
                    childEnd++;
                }
                children.emplace(child->name, std::move(child));
                break;
            }
            case Command::Type::ChildRemove: {
                const std::string& childName = std::get<std::string>(cmd.data);
                auto it = children.find(childName);
                if (it == children.end()) break; // child not found, ignore
                Node* removed = it->second.get();
                auto cacheIt = std::find(childrenCache.begin(), childrenCache.begin() + childEnd, removed);
                CB_ASSERT(cacheIt != childrenCache.begin() + childEnd);
                std::swap(*cacheIt, childrenCache[--childEnd]);
                children.erase(it);
                break;
            }
            case Command::Type::ComponentAdd: {
                auto component = std::move(std::get<std::unique_ptr<Component>>(cmd.data));
                TypeID typeID = component->getType();
                auto it = components.find(typeID);
                if (it != components.end()) break; // component of the same type already exists, ignore
                if (componentEnd < componentsCache.size()) {
                    componentsCache[componentEnd++] = component.get();
                } else {
                    componentsCache.push_back(component.get());
                    componentEnd++;
                }
                components.emplace(typeID, std::move(component));
                break;
            }
            case Command::Type::ComponentRemove: {
                TypeID typeID = std::get<TypeID>(cmd.data);
                auto it = components.find(typeID);
                if (it == components.end()) break; // component not found, ignore
                Component* removed = it->second.get();
                auto cacheIt = std::find(componentsCache.begin(), componentsCache.begin() + componentEnd, removed);
                CB_ASSERT(cacheIt != componentsCache.begin() + componentEnd);
                std::swap(*cacheIt, componentsCache[--componentEnd]);
                components.erase(it);
                break;
            }
        }
    }
    childrenCache.resize(childEnd);
    componentsCache.resize(componentEnd);
    commandQueue.clear();
}

}  // namespace Cube

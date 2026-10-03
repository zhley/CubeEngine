#include "EditorNodeAccess.h"

#include <algorithm>

#include "Cube/Reflection/ClassRegistry.h"
#include "Cube/Script/ScriptComponent.h"

Cube::Node* EditorNodeAccess::addChild(Cube::Node& parent, std::unique_ptr<Cube::Node> child) {
    if(!child || child.get() == &parent) {
        return nullptr;
    }
    if(parent.children.contains(child->name)) {
        return nullptr;
    }
    Cube::Node* result = child.get();
    result->parent = &parent;
    parent.childrenCache.push_back(result);
    parent.children.emplace(child->name, std::move(child));
    return result;
}

Cube::Node* EditorNodeAccess::addChild(Cube::Node& parent, const std::string& name) {
    return addChild(parent, std::make_unique<Cube::Node>(name));
}

void EditorNodeAccess::removeChild(Cube::Node& parent, Cube::Node* child) {
    if(!child) {
        return;
    }
    auto childIt = parent.children.find(child->name);
    if(childIt == parent.children.end() || childIt->second.get() != child) {
        return;
    }
    auto cacheIt = std::find(parent.childrenCache.begin(), parent.childrenCache.end(), child);
    if(cacheIt == parent.childrenCache.end()) {
        return;
    }
    parent.childrenCache.erase(cacheIt);
    parent.children.erase(childIt);
}

void EditorNodeAccess::removeChild(Cube::Node& parent, const std::string& name) {
    auto childIt = parent.children.find(name);
    if(childIt != parent.children.end()) {
        removeChild(parent, childIt->second.get());
    }
}

Cube::Component* EditorNodeAccess::addComponent(Cube::Node& node, std::unique_ptr<Cube::Component> component) {
    if(!component) {
        return nullptr;
    }
    Cube::Component* result = component.get();
    result->node = &node;
    if(component->getType() == Cube::getTypeID<Cube::ScriptComponent>()) {
        std::unique_ptr<Cube::ScriptComponent> scriptComp(static_cast<Cube::ScriptComponent*>(component.release()));
        if(node.scriptComps.contains(scriptComp->getName())) {
            return nullptr;
        }
        node.scriptComps.emplace(scriptComp->getName(), std::move(scriptComp));
    } else {
        const Cube::TypeID typeID = component->getType();
        if(node.components.contains(typeID)) {
            return nullptr;
        }
        node.components.emplace(typeID, std::move(component));
    }
    node.componentsCache.push_back(result);
    return result;
}

void EditorNodeAccess::removeComponent(Cube::Node& node, Cube::TypeID typeID) {
    auto compIt = node.components.find(typeID);
    if(compIt == node.components.end()) {
        return;
    }
    Cube::Component* removed = compIt->second.get();
    auto cacheIt = std::find(node.componentsCache.begin(), node.componentsCache.end(), removed);
    if(cacheIt != node.componentsCache.end()) {
        node.componentsCache.erase(cacheIt);
    }
    node.components.erase(compIt);
}

void EditorNodeAccess::removeComponent(Cube::Node& node, const std::string& typeName) {
    if(Cube::Class* classInfo = Cube::ClassRegistry::get().getClass(typeName)) {
        removeComponent(node, classInfo->getTypeID());
        return;
    }
    auto compIt = node.scriptComps.find(typeName);
    if(compIt == node.scriptComps.end()) {
        return;
    }
    Cube::Component* removed = compIt->second.get();
    auto cacheIt = std::find(node.componentsCache.begin(), node.componentsCache.end(), removed);
    if(cacheIt != node.componentsCache.end()) {
        node.componentsCache.erase(cacheIt);
    }
    node.scriptComps.erase(compIt);
}

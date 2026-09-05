#include "Node.h"

#include <algorithm>
#include <functional>

#include "Cube/Core/Log.h"
#include "Cube/Reflection/Serializer.h"
#include "Cube/Resource/NodeTree.h"

namespace Cube {

    void Node::update(float delta) {
        for(auto& component : components) {
            component->update(delta);
        }
        processAddAndDestroy();
        processStart();

        // Iterate by index: children may be attached while the subtree updates.
        for(size_t i = 0; i < children.size(); ++i) {
            children[i]->update(delta);
        }
        processChildDestroy();
    }

    Component* Node::addComponent(std::unique_ptr<Component> component) {
        if(componentsMap.find(component->getType()) != componentsMap.end()) {
            CB_CORE_ERROR("Node::addComponent(): component of type '{}' already exists", ClassRegistry::get().getClass(component->getType())->getName());
            return nullptr;
        }
        Component* ptr = component.get();
        ptr->node = this;
        pendingAdd.push_back(std::move(component));
        addOrDestroy.push_back(0);
        return ptr;
    }

    Component* Node::addComponent(const std::string& typeName) {
        Class* classInfo = ClassRegistry::get().getClass(typeName);
        if(!classInfo) {
            CB_CORE_ERROR("Node::addComponent(): Unknown component type '{}'", typeName);
            return nullptr;
        }
        std::unique_ptr<Component> component(classInfo->createInstance().moveToBase<Component>());
        return addComponent(std::move(component));
    }

    void Node::removeComponent(TypeID typeID) {
        auto it = componentsMap.find(typeID);
        if(it == componentsMap.end()) {
            CB_CORE_ERROR("Node::removeComponent(): component of type '{}' does not exist", ClassRegistry::get().getClass(typeID)->getName());
            return;
        }
        pendingDestroy.push_back(typeID);
        addOrDestroy.push_back(1);
    }

    void Node::removeComponent(const std::string& typeName) {
        Class* classInfo = ClassRegistry::get().getClass(typeName);
        if(!classInfo) {
            CB_CORE_ERROR("Node::removeComponent(): Unknown component type '{}'", typeName);
            return;
        }
        removeComponent(classInfo->getTypeID());
    }

    Component* Node::getComponent(const std::string& typeName) const {
        Class* classInfo = ClassRegistry::get().getClass(typeName);
        if(!classInfo) {
            CB_CORE_ERROR("Node::getComponent(): Unknown component type '{}'", typeName);
            return nullptr;
        }
        TypeID typeID = classInfo->getTypeID();
        auto it = componentsMap.find(typeID);
        if(it == componentsMap.end()) {
            CB_CORE_ERROR("Node::getComponent(): component of type '{}' does not exist", typeName);
            return nullptr;
        }
        return it->second;
    }

    bool Node::hasComponent(const std::string& typeName) const {
        Class* classInfo = ClassRegistry::get().getClass(typeName);
        if(!classInfo) {
            CB_CORE_ERROR("Node::hasComponent(): Unknown component type '{}'", typeName);
            return false;
        }
        TypeID typeID = classInfo->getTypeID();
        return componentsMap.find(typeID) != componentsMap.end();
    }

    Node* Node::addChild(const std::string& name) {
        std::unique_ptr<Node> child = std::make_unique<Node>(name);
        child->parent = this;
        Node* childPtr = child.get();
        children.push_back(std::move(child));
        return childPtr;
    }

    Node* Node::addChild(std::unique_ptr<Node> child) {
        child->parent = this;
        Node* childPtr = child.get();
        children.push_back(std::move(child));
        return childPtr;
    }

    void Node::removeChild(Node* child) {
        auto it = std::find_if(children.begin(), children.end(), [child](const std::unique_ptr<Node>& c) {
            return c.get() == child;
        });
        if(it != children.end()) {
            (*it)->destroy();
        }
    }

    void Node::removeChild(const std::string& name) {
        auto it = std::find_if(children.begin(), children.end(), [&name](const std::unique_ptr<Node>& c) {
            return c->getName() == name;
        });
        if(it != children.end()) {
            (*it)->destroy();
        }
    }

    Node* Node::findChild(const std::string& name) const {
        for(const auto& child : children) {
            if(child->getName() == name) {
                return child.get();
            }
            if(Node* found = child->findChild(name)) {
                return found;
            }
        }
        return nullptr;
    }

    std::vector<Node*> Node::getAllNodes() const {
        std::vector<Node*> result;
        std::function<void(const Node*)> traverse = [&](const Node* node) {
            result.push_back(const_cast<Node*>(node));
            for(const auto& child : node->children) {
                traverse(child.get());
            }
        };
        traverse(this);
        return result;
    }

    Node* Node::addChildFromTree(NodeTree* nodeTree) {
        if(!nodeTree) {
            CB_CORE_ERROR("Node::addChildFromTree(): node tree is null");
            return nullptr;
        }
        std::unique_ptr<Node> subtree = nodeTree->instantiate();
        if(!subtree) {
            CB_CORE_ERROR("Node::addChildFromTree(): failed to instantiate the node tree");
            return nullptr;
        }
        return addChild(std::move(subtree));
    }

    void Node::deserialize(const nlohmann::json& data) {
        if(data.contains("name")) {
            name = data["name"];
        }
        if(data.contains("transform")) {
            const nlohmann::json& tr = data["transform"];
            transform.pos = {tr["pos"][0], tr["pos"][1]};
            transform.rotation = tr["rotation"];
            transform.scale = {tr["scale"][0], tr["scale"][1]};
        }
        if(data.contains("components")) {
            for(auto& c : data["components"]) {
                std::string typeName = c["type"];
                Class* classInfo = ClassRegistry::get().getClass(typeName);
                if(!classInfo) {
                    CB_CORE_ERROR("Node::deserialize(): Unknown component type '{}'", typeName);
                    continue;
                }
                Any component = Serializer::get().deserialize(classInfo->getTypeID(), c);
                Component* compPtr = component.moveToBase<Component>();
                addComponent(std::unique_ptr<Component>(compPtr));
            }
        }
        if(data.contains("children")) {
            for(auto& childData : data["children"]) {
                std::string childName = childData.value("name", "");
                Node* child = addChild(childName);
                child->deserialize(childData);
            }
        }
    }

    nlohmann::json Node::serialize() const {
        nlohmann::json data;
        data["name"] = name;
        nlohmann::json tr;
        tr["pos"] = {transform.pos.x, transform.pos.y};
        tr["rotation"] = transform.rotation;
        tr["scale"] = {transform.scale.x, transform.scale.y};
        data["transform"] = tr;
        data["components"] = nlohmann::json::array();
        for(const auto& [typeID, component] : componentsMap) {
            nlohmann::json c = Serializer::get().serialize(typeID, Any(component));
            c["type"] = ClassRegistry::get().getClass(typeID)->getName();
            data["components"].push_back(c);
        }
        data["children"] = nlohmann::json::array();
        for(const auto& child : children) {
            data["children"].push_back(child->serialize());
        }
        return data;
    }

    void Node::processAddAndDestroy() {
        int addIndex = 0;
        int destroyIndex = 0;
        for(auto& action : addOrDestroy) {
            if(action == 0) { // add
                auto& component = pendingAdd[addIndex++];
                Component* ptr = component.get();
                componentsMap[ptr->getType()] = ptr;
                components.push_back(std::move(component));
                pendingStart.push_back(ptr);
            } else { // destroy
                TypeID typeID = pendingDestroy[destroyIndex++];
                auto it = componentsMap.find(typeID);
                Component* compPtr = it->second;
                componentsMap.erase(it);
                components.erase(std::remove_if(components.begin(), components.end(), [compPtr](const std::unique_ptr<Component>& c) {
                    return c.get() == compPtr;
                }), components.end());
                pendingStart.erase(std::remove(pendingStart.begin(), pendingStart.end(), compPtr), pendingStart.end());
            }
        }
        CB_ASSERT(addIndex == pendingAdd.size() && destroyIndex == pendingDestroy.size() && "Node::processAddAndDestroy(): add and destroy count mismatch");
        pendingAdd.clear();
        pendingDestroy.clear();
        addOrDestroy.clear();
    }

    void Node::processStart() {
        for(Component* c : pendingStart) {
            c->start();
        }
        pendingStart.clear();
    }

    void Node::processChildDestroy() {
        auto end = std::remove_if(children.begin(), children.end(), [](const std::unique_ptr<Node>& child) {
            return !child->isAlive();
        });
        children.erase(end, children.end());
        for(auto& child : children) {
            child->processChildDestroy();
        }
    }

}  // namespace Cube

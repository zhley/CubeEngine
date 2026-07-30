#include "Entity.h"

#include "Cube/Core/Log.h"
#include "Cube/Scene/Component.h"
#include "Cube/Reflection/Serializer.h"

namespace Cube {

    void Entity::update(float delta) {
        for(auto& component : components) {
            component->update(delta);
        }
        processAddAndDestroy();
        processStart();

        for(auto& child : children) {
            child->update(delta);
        }
    }

    Component* Entity::addComponent(std::unique_ptr<Component> component) {
        if(componentsMap.find(component->getType()) != componentsMap.end()) {
            CB_CORE_ERROR("Entity::addComponent(): component of type '{}' already exists", ClassRegistry::get().getClass(component->getType())->getName());
            return nullptr;
        }
        Component* ptr = component.get();
        ptr->entity = this;
        pendingAdd.push_back(std::move(component));
        addOrDestroy.push_back(0);
        return ptr;
    }

    void Entity::deserialize(const nlohmann::json& data) {
        name = data["name"];
        auto tr = data["transform"];
        transform.pos = {tr["pos"][0], tr["pos"][1]};
        transform.rotation = tr["rotation"];
        transform.scale = {tr["scale"][0], tr["scale"][1]};
        for(auto& c : data["components"]) {
            std::string typeName = c["type"];
            Class* classInfo = ClassRegistry::get().getClass(typeName);
            if(!classInfo) {
                CB_CORE_ERROR("Entity::deserialize(): Unknown component type '{}'", typeName);
                continue;
            }
            Any component = Serializer::get().deserialize(classInfo->getTypeID(), c);
            Component* compPtr = component.moveToBase<Component>();
            addComponent(std::unique_ptr<Component>(compPtr));
        }
        for(auto& childData : data["children"]) {
            Entity* child = addChild(childData["name"]);
            child->deserialize(childData);
        }
    }

    nlohmann::json Entity::serialize() const {
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

    void Entity::processAddAndDestroy() {
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
        CB_ASSERT(addIndex == pendingAdd.size() && destroyIndex == pendingDestroy.size() && "Entity::processAddAndDestroy(): add and destroy count mismatch");
        pendingAdd.clear();
        pendingDestroy.clear();
        addOrDestroy.clear();
    }

    void Entity::processStart() {
        for(Component* c : pendingStart) {
            c->start();
        }
        pendingStart.clear();
    }

    Entity* Entity::addChild(const std::string& name) {
        std::unique_ptr<Entity> child = std::make_unique<Entity>(name);
        child->parent = this;
        child->scene = scene;
        Entity* childPtr = child.get();
        children.push_back(std::move(child));
        return childPtr;
    }

    void Entity::removeChild(Entity* child) {
        auto it = std::find_if(children.begin(), children.end(), [child](const std::unique_ptr<Entity>& c) {
            return c.get() == child;
        });
        if(it != children.end()) {
            (*it)->destroy();
        }
    }

    void Entity::removeChild(const std::string& name) {
        auto it = std::find_if(children.begin(), children.end(), [&name](const std::unique_ptr<Entity>& c) {
            return c->getName() == name;
        });
        if(it != children.end()) {
            (*it)->destroy();
        }
    }

    void Entity::processChildDestroy() {
        auto end = std::remove_if(children.begin(), children.end(), [](const std::unique_ptr<Entity>& child) {
            return !child->isAlive();
        });
        children.erase(end, children.end());
        for(auto& child : children) {
            child->processChildDestroy();
        }
    }

}  // namespace Cube
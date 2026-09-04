#include "Scene.h"

#include <fstream>

#include "json.hpp"

#include "SpriteRender.h"

namespace Cube {

    Scene::Scene(const std::string& name) : name(name) {
        rootEntity = std::make_unique<Entity>("Root");
        rootEntity->scene = this;
    }

    Scene::Scene(const Cube::Path& sceneFilePath) {
        rootEntity = std::make_unique<Entity>("Root");
        rootEntity->scene = this;
        
        std::ifstream file(sceneFilePath.string());
        if(!file.is_open()) {
            CB_CORE_ERROR("Scene::Scene(): Failed to open scene file '{}'", sceneFilePath);
            return;
        }
        nlohmann::json data;
        file >> data;
        file.close();
        name = data["name"];
        rootEntity->deserialize(data["rootEntity"]);
    }

    void Scene::update(float delta) {
        rootEntity->update(delta);
        processDestroy();
    }

    Entity* Scene::createEntity(const std::string& name) {
        return rootEntity->addChild(name);
    }

    void Scene::destroyEntity(const std::string& name) {
        rootEntity->removeChild(name);
    }

    void Scene::destroyEntity(Entity* entity) {
        rootEntity->removeChild(entity);
    }

    std::vector<Entity*> Scene::getAllEntities() const {
        std::vector<Entity*> result;
        std::function<void(const Entity*)> traverse = [&](const Entity* entity) {
            result.push_back(const_cast<Entity*>(entity));
            for(const auto& child : entity->children) {
                traverse(child.get());
            }
        };
        traverse(rootEntity.get());
        return result;
    }

    Entity* Scene::getEntity(const std::string& name) const {
        std::function<Entity*(const Entity*)> traverse = [&](const Entity* entity) -> Entity* {
            if(entity->getName() == name) {
                return const_cast<Entity*>(entity);
            }
            for(const auto& child : entity->children) {
                Entity* found = traverse(child.get());
                if(found) {
                    return found;
                }
            }
            return nullptr;
        };
        return traverse(rootEntity.get());
    }

    void Scene::serialize(const Cube::Path& sceneFilePath) const {
        nlohmann::json data;
        data["name"] = name;
        data["rootEntity"] = rootEntity->serialize();
        std::ofstream file(sceneFilePath.string());
        if(!file.is_open()) {
            CB_CORE_ERROR("Scene::serialize(): Failed to open scene file '{}'", sceneFilePath);
            return;
        }
        file << data.dump(4);
        file.close();
    }

    const std::string& Scene::getName() const {
        return name;
    }

    void Scene::processDestroy() {
        rootEntity->processChildDestroy();
    }

    void Scene::addRenderableEntity(Entity* entity) {
        renderableEntities.push_back(entity);
    }

    void Scene::removeRenderableEntity(Entity* entity) {
        auto it = std::find(renderableEntities.begin(), renderableEntities.end(), entity);
        if(it != renderableEntities.end()) {
            renderableEntities.erase(it);
        }
    }

    void Scene::addCamera(Entity* entity) {
        cameras.push_back(entity);
    }

    void Scene::removeCamera(Entity* entity) {
        auto it = std::find(cameras.begin(), cameras.end(), entity);
        if(it != cameras.end()) {
            cameras.erase(it);
        }
    }

    void Scene::sortRenderableEntities() {
        std::sort(renderableEntities.begin(), renderableEntities.end(), [](const Entity* a, const Entity* b) {
            SpriteRender* spriteA = a->getComponent<SpriteRender>();
            SpriteRender* spriteB = b->getComponent<SpriteRender>();
            if(spriteA->order != spriteB->order) {
                return spriteA->order < spriteB->order;
            }
            return (spriteA->sprite && spriteA->sprite->getTexture() ? spriteA->sprite->getTexture()->getId() : -1) < (spriteB->sprite && spriteB->sprite->getTexture() ? spriteB->sprite->getTexture()->getId() : -1);
        });
    }
}  // namespace Cube
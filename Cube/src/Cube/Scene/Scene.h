#pragma once
#include <memory>
#include <vector>
#include "Entity.h"

namespace Cube {

    // GameObject-Component
    class Scene {
    public:
        Scene(const std::string& name, bool tagNoSceneFile) : name(name){}
        Scene(const std::string& sceneFilePath);
        virtual ~Scene() = default;

        void update(float delta);

        Entity* createEntity(const std::string& name);
        void destroyEntity(const std::string& name);
        void destroyEntity(Entity* entity);

        Entity* getRootEntity() const { return rootEntity.get(); }
        std::vector<Entity*> getAllEntities() const;
        Entity* getEntity(const std::string& name) const;

        template<typename... Types>
        std::vector<Entity*> getEntitiesWith() const {
            static_assert((std::is_base_of_v<Component, Types> && ...));
            std::vector<Entity*> result;
            for(const auto& entity : getAllEntities()) {
                bool hasAll = true;
                ((hasAll = hasAll && entity->hasComponent<Types>()), ...);
                if(hasAll) {
                    result.push_back(entity);
                }
            }

            return result;
        }

        void serialize(const std::string& sceneFilePath) const;

        const std::string& getName() const;

        const std::vector<Entity*>& getSortedRenderableEntities() { sortRenderableEntities(); return renderableEntities; }
        const std::vector<Entity*>& getCameras() const { return cameras; }
        void addRenderableEntity(Entity* entity);
        void removeRenderableEntity(Entity* entity);
        void addCamera(Entity* entity);
        void removeCamera(Entity* entity);

    private:
        std::string name;
        std::unique_ptr<Entity> rootEntity = std::make_unique<Entity>("Root");

        std::vector<Entity*> renderableEntities; 
        std::vector<Entity*> cameras;

        void processDestroy();
        void sortRenderableEntities();
    };

}
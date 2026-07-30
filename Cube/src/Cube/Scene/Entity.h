#pragma once

#include <memory>
#include <vector>

#include "json.hpp"
#include "glm/glm.hpp"

#include "Component.h"
#include "Transform.h"
#include "Cube/Scene/Component.h"
#include "Cube/Reflection/ClassRegistry.h"
#include "Cube/Reflection/Type.h"




namespace Cube {

	// GameObject in scene
	class Entity {
	public:
		friend class Scene;

		Entity() = default;
		Entity(const std::string& name) : name(name) {}
		~Entity() = default;

		void update(float delta);

		Component* addComponent(std::unique_ptr<Component> component);

		template<typename T>
		T* addComponent() {
			static_assert(std::is_base_of_v<Component, T>);
			return static_cast<T*>(addComponent(std::make_unique<T>()));
        }

		void removeComponent(TypeID typeID) {
			auto it = componentsMap.find(typeID);
			if(it == componentsMap.end()) {
				CB_CORE_ERROR("Entity::removeComponent(): component of type '{}' does not exist", ClassRegistry::get().getClass(typeID)->getName());
				return;
			}
			pendingDestroy.push_back(typeID);
			addOrDestroy.push_back(1);
		}

		template<typename T>
		void removeComponent() {
			static_assert(std::is_base_of_v<Component, T>);
            removeComponent(getTypeID<T>());
		}

		template<typename T>
		T* getComponent() const {
			static_assert(std::is_base_of_v<Component, T>);
			TypeID typeID = getTypeID<T>();
			auto it = componentsMap.find(typeID);
			if(it == componentsMap.end()) {
				CB_CORE_ERROR("Entity::getComponent<T>(): component of type '{}' does not exist", ClassRegistry::get().getClass<T>()->getName());
				return nullptr;
			}
			return static_cast<T*>(it->second);
        }

		const std::vector<std::unique_ptr<Component>>& getComponents() const { return components; }
        // const std::unordered_map<TypeID, Component*>& getComponentsMap() const { return componentsMap; }

		template<typename T>
		bool hasComponent() const {
			static_assert(std::is_base_of_v<Component, T>);
			TypeID typeID = getTypeID<T>();
			return componentsMap.find(typeID) != componentsMap.end();
        }

		bool isAlive() const { return alive; }
        void destroy() { alive = false; }

		const std::string& getName() const { return name; }
		void setName(const std::string& name) { this->name = name; }
        Transform& getTransform() { return transform; }
		Entity* getParent() const { return parent; }
		Scene* getScene() const { return scene; }

		const std::vector<std::unique_ptr<Entity>>& getChildren() const { return children; }
		Entity* addChild(const std::string& name);
		void removeChild(Entity* child);
		void removeChild(const std::string& name);

		void deserialize(const nlohmann::json& data);
        nlohmann::json serialize() const;

	private:
        std::string name;  // Temporarily used as the unique identifier
		bool alive = true;
        std::vector<std::unique_ptr<Component>> components;  // for iteration
        std::unordered_map<TypeID, Component*> componentsMap;  // for lookup
		Transform transform = Transform(this);
		
		Entity* parent = nullptr;
		std::vector<std::unique_ptr<Entity>> children;
		Scene* scene = nullptr;

		std::vector<std::unique_ptr<Component>> pendingAdd;
		std::vector<TypeID> pendingDestroy;
		std::vector<char> addOrDestroy; // 0 for add, 1 for destroy  denote the order of add and destroy
		std::vector<Component*> pendingStart;

		void processStart();
		void processAddAndDestroy();
		void processChildDestroy();
	};

}
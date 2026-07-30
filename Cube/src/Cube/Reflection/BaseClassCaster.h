#pragma once

#include "Cube/Core/Log.h"
#include "Type.h"

namespace Cube {

	template<typename BaseType>
    class BaseClassCaster {
    public:
        using ToBaseCast = std::function<BaseType*(void*)>;

        template<typename DerivedType>
        static void registerCast() {
            static_assert(std::is_base_of_v<BaseType, DerivedType>, "DerivedType must inherit from BaseType");
            if(registry.find(getTypeID<DerivedType>()) != registry.end()) {
                CB_CORE_WARN("Reflection/BaseClassCaster::registerCast(): DerivedType {} is already registered");
                return;
            }
            registry[getTypeID<DerivedType>()] = [](void* derivedPtr) -> BaseType* {
                return static_cast<BaseType*>(static_cast<DerivedType*>(derivedPtr));
            };
        }

        static BaseType* cast(TypeID typeID, void* derivedPtr) {
            if(!derivedPtr) {
                CB_CORE_ERROR("Reflection/BaseClassCaster::cast(): derivedPtr is nullptr");
                return nullptr;
            }
            auto it = registry.find(typeID);
            if(it == registry.end()) {
                CB_CORE_ERROR("Reflection/BaseClassCaster::cast(): No cast registered for TypeID {}", typeID);
                return nullptr;
            }
            return it->second(derivedPtr);
        }

	private:
        static inline std::unordered_map<TypeID, ToBaseCast> registry;
    };
	
}
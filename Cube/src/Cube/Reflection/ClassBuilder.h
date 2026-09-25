#pragma once

#include "BaseClassCaster.h"
#include "Class.h"
#include "ClassRegistry.h"
#include "Serializer.h"

namespace Cube {

	template<typename T>
	class ClassBuilder final{
	public:
		ClassBuilder(const std::string& name) {
		    classInfo = ClassRegistry::get().registerClass<T>(name);
		}

		template<typename PropertyType>
		ClassBuilder& property(const std::string& name, PropertyType T::*memberPtr) {
		    size_t offset = reinterpret_cast<size_t>(&(static_cast<T*>(nullptr)->*memberPtr));
			classInfo->addProperty(name, getTypeID<PropertyType>(), offset, sizeof(PropertyType), [memberPtr](const void* obj) {
			    const T* instance = static_cast<const T*>(obj);
				return Any(instance->*memberPtr);
			}, [memberPtr](void* obj, const Any& value) {
				if constexpr(!std::is_copy_assignable_v<PropertyType>) {
                    CB_CORE_ERROR("Reflection: Attempting to set value to a non-assignable property.");
				} else {
					T* instance = static_cast<T*>(obj);
					instance->*memberPtr = value.as<PropertyType>();
				}
			}, [memberPtr](void* obj, Any&& value) {
			    if constexpr (!std::is_move_assignable_v<PropertyType>) {
					CB_CORE_ERROR("Reflection: Attempting to move-assign value to a non-move-assignable property.");
				} else {
					T* instance = static_cast<T*>(obj);
					instance->*memberPtr = std::move(*value.move<PropertyType>());
				}
			});
			return *this;
		}

		template<typename PropertyType>
		ClassBuilder& property(const std::string& name, PropertyType T::*memberPtr, Property::Getter getter, Property::Setter setter, Property::MoveSetter moveSetter) {
			size_t offset = reinterpret_cast<size_t>(&(static_cast<T*>(nullptr)->*memberPtr));
            classInfo->addProperty(name, getTypeID<PropertyType>(), offset, sizeof(PropertyType), std::move(getter), std::move(setter), std::move(moveSetter));
			return *this;
		}

		template<typename ReturnType, typename... Args>
		ClassBuilder& method(const std::string& name, ReturnType(T::*methodPtr)(Args...)) {
		    classInfo->addMethod(name, getTypeID<ReturnType>(), {getTypeID<Args>()...}, [methodPtr](void* obj, const std::vector<Any>& args) {
		        T* instance = static_cast<T*>(obj);
				if(args.size() != sizeof...(Args)) {
					CB_CORE_ERROR("Reflection: Mismatched number of function parameter");
				    CB_ASSERT(0);
					return Any();
				}
				return invokeImpl(instance, methodPtr, args, std::make_index_sequence<sizeof...(Args)>{});
		    });
			return *this;
		}

		template<typename ReturnType, typename... Args>
		ClassBuilder& method(const std::string& name, ReturnType(T::*methodPtr)(Args...) const) {
		    classInfo->addMethod(name, getTypeID<ReturnType>(), {getTypeID<Args>()...}, [methodPtr](void* obj, const std::vector<Any>& args) {
		        const T* instance = static_cast<const T*>(obj);
				if(args.size() != sizeof...(Args)) {
					CB_CORE_ERROR("Reflection: Mismatched number of function parameter");
				    CB_ASSERT(0);
					return Any();
				}
				return invokeImpl(instance, methodPtr, args, std::make_index_sequence<sizeof...(Args)>{});
		    });
			return *this;
		}

		template<typename BaseType>
		ClassBuilder& base() {
			BaseClassCaster<BaseType>::template registerCast<T>();
		    classInfo->setBase(getTypeID<BaseType>());
			return *this;
		}

		ClassBuilder& serializer() {
		    registerSerializer<T>();
			return *this;
		}

		Class* GetClass() const {
		    return classInfo;
		}

	private:
		Class* classInfo;

		template<typename ReturnType, typename... Args, size_t... Idx>
		static Any invokeImpl(T* instance, ReturnType(T::*methodPtr)(Args...), const std::vector<Any>& args, std::index_sequence<Idx...>) {
		    if constexpr (std::is_void_v<ReturnType>) {
		        (instance->*methodPtr)(args[Idx].template as<std::decay_t<Args>>()...);
				return Any();
		    }else {
				return Any((instance->*methodPtr)(args[Idx].template as<std::decay_t<Args>>()...));
		    }
		}

		template<typename ReturnType, typename... Args, size_t... Idx>
		static Any invokeImpl(const T* instance, ReturnType(T::*methodPtr)(Args...) const, const std::vector<Any>& args, std::index_sequence<Idx...>) {
		    if constexpr (std::is_void_v<ReturnType>) {
		        (instance->*methodPtr)(args[Idx].template as<std::decay_t<Args>>()...);
				return Any();
		    }else {
				return Any((instance->*methodPtr)(args[Idx].template as<std::decay_t<Args>>()...));
		    }
		}
	};

}

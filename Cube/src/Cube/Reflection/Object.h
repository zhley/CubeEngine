#pragma once

#include "Class.h"
#include "ClassRegistry.h"

namespace Cube {

	// wrapper
	class Object {
	public:
		template<typename T>
		Object(T* ptr) : ptr(ptr), classInfo(ClassRegistry::get().getClass<T>()){}

		Object(void* ptr, Class* classInfo) : ptr(ptr), classInfo(classInfo){}
		bool isValid() { return ptr && classInfo; }

		template<typename T>
		T get(const std::string& name) {
			T res;
			if(!isValid()) {
			    CB_CORE_ERROR("Reflection/Object::get(): invalid object");
				return res;
			}
			Property* property = classInfo->getProperty(name);
			if(!property) return res;
			return property->getValue(ptr).as<T>();
		}

		template<typename T>
		bool set(const std::string& name, const T& value) {
			if(!isValid()) {
				CB_CORE_ERROR("Reflection/Object::set(): invalid object");
				return false;
			}
			Property* property = classInfo->getProperty(name);
			if(!property) return false;
			property->setValue(ptr, Any(value));
			return true;
		}

		void* getPtr() const { return ptr; }
		Class* getClassInfo() const { return classInfo; }

	private:
		void* ptr;
		Class* classInfo;
	};

}
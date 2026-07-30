#pragma once

#include "Class.h"
#include "Type.h"

namespace Cube {

	class ClassRegistry final{
	public:
		static ClassRegistry& get() {
		    static ClassRegistry instance;
			return instance;
		}

		template<typename T>
		Class* registerClass(const std::string& name) {
		    TypeID id = getTypeID<T>();
			auto it = classes.find(id);
			if(it != classes.end()) {
			    return it->second.get();
			}

			auto cls = std::make_unique<Class>(name, id, sizeof(T), [] {
			    return static_cast<void*>(new T());
			}, [](void* ptr) {
			    delete static_cast<T*>(ptr);
			});
			Class* pointer = cls.get();
			classes[id] = std::move(cls);
			nameToClass[name] = pointer;
			return pointer;
		}

		Class* getClass(TypeID id) {
		    auto it = classes.find(id);
			return it != classes.end() ? it->second.get() : nullptr;
		}

		Class* getClass(const std::string& name) {
		    auto it = nameToClass.find(name);
			return it != nameToClass.end() ? it->second : nullptr;
		}

		template<typename T>
		Class* getClass() {
		    return getClass(getTypeID<T>());
		}

		std::vector<Class*> getAllClasses() {
		    std::vector<Class*> res;
			res.reserve(classes.size());
            for(auto& c : classes) {
			    res.push_back(c.second.get());
			}
			return res;
		}

	private:
		ClassRegistry() = default;
		~ClassRegistry() = default;

		std::unordered_map<TypeID, std::unique_ptr<Class>> classes;
		std::unordered_map<std::string, Class*> nameToClass;
	};

}
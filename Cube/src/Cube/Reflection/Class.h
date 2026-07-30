#pragma once

#include "Any.h"
#include "BaseClassCaster.h"
#include "Type.h"

namespace Cube {

    class Property final {
    public:
        using Getter = std::function<Any(const void*)>;
        using Setter = std::function<void(void*, const Any&)>;
        using MoveSetter = std::function<void(void*, Any&&)>;
        Property(const std::string& name, TypeID typeID, size_t offset, size_t size, Getter getter, Setter setter, MoveSetter moveSetter)
            : name(name), typeID(typeID), offset(offset), size(size), getter(std::move(getter)), setter(std::move(setter)), moveSetter(std::move(moveSetter)) {}

        const std::string& getName() const { return name; }
        TypeID getTypeID() const { return typeID; }
        size_t getOffset() const { return offset; }
        size_t getSize() const { return size; }

        Any getValue(const void* instance) const {            
            return getter(instance);
        }
                    
        void setValue(void* instance, const Any& value) const {
            setter(instance, value);
        }

        void setValue(void* instance, Any&& value) const {
            moveSetter(instance, std::move(value));
        }

    private:
        std::string name;
        TypeID typeID;
        size_t offset;
        size_t size;
        Getter getter;
        Setter setter;
        MoveSetter moveSetter;
    };

    class Method final {
    public:
        using Invoker = std::function<Any(void*, const std::vector<Any>&)>;

        Method(const std::string& name, TypeID returnTypeID, const std::vector<TypeID>& parameters, Invoker invoker)
            : name(name), returnTypeID(returnTypeID), parameters(parameters), invoker(std::move(invoker)) {}

        const std::string& getName() const { return name; }
        TypeID getReturnTypeID() const { return returnTypeID; }
        const std::vector<TypeID>& getParameters() const { return parameters; }

        Any invoke(void* instance, const std::vector<Any>& args) const {
            return invoker(instance, args);
        }

    private:
        std::string name;
        TypeID returnTypeID;
        std::vector<TypeID> parameters;
        Invoker invoker;
    };

    class Class final {
    public:
        using Constructor = std::function<void*()>;
        using Destructor = std::function<void(void*)>;

        Class(const std::string& name, TypeID typeID, size_t size, Constructor constructor, Destructor destructor)
            : name(name), typeID(typeID), baseTypeID(0), size(size), constructor(std::move(constructor)), destructor(std::move(destructor)) {}

        Any createInstance() const {
            return {constructor(), typeID, destructor};
        }

        void addProperty(const std::string& name, TypeID typeID, size_t offset, size_t size, Property::Getter getter, Property::Setter setter, Property::MoveSetter moveSetter) {
            properties.emplace(name, std::make_unique<Property>(name, typeID, offset, size, std::move(getter), std::move(setter), std::move(moveSetter)));
        }

        void addMethod(const std::string& name, TypeID returnTypeID, const std::vector<TypeID>& parameters, const Method::Invoker& invoker) {
            methods.emplace(name, std::make_unique<Method>(name, returnTypeID, parameters, invoker));
        }

        void setBase(TypeID typeID) { baseTypeID = typeID; }
        const std::string& getName() const { return name; }
        TypeID getTypeID() const { return typeID; }
        TypeID getBaseTypeID() const { return baseTypeID; }
        size_t getSize() const { return size; }

        std::vector<const Property*> getAllProperties() const {
            std::vector<const Property*> props;
            props.reserve(properties.size());
            for (const auto& [name, prop] : properties) {
                props.push_back(prop.get());
            }
            return props;
        }

        Property* getProperty(const std::string& name) const {
            auto it = properties.find(name);
            if (it != properties.end()) {
                return it->second.get();
            }
            CB_CORE_ERROR("Reflection: Property \"{}\" in class {} not found.", name, this->name);
            return nullptr;
        }

        std::vector<const Method*> getAllMethods() const {
            std::vector<const Method*> ms;
            ms.reserve(methods.size());
            for (const auto& [name, m] : methods) {
                ms.push_back(m.get());
            }
            return ms;
        }

        Method* getMethod(const std::string& name) const {
            auto it = methods.find(name);
            if (it != methods.end()) {
                return it->second.get();
            }
            CB_CORE_ERROR("Reflection: Method \"{}\" in class {} not found.", name, this->name);
            return nullptr;
        }

        template<typename BaseType>
        BaseType* castToBase(void* derivedPtr) const {
            return BaseClassCaster<BaseType>::cast(typeID, derivedPtr);
        }

    private:
        std::string name;
        TypeID typeID;
        TypeID baseTypeID;  // type of the base class
        size_t size;
        Constructor constructor;
        Destructor destructor;
        std::unordered_map<std::string, std::unique_ptr<Property>> properties;
        std::unordered_map<std::string, std::unique_ptr<Method>> methods;
    };

}  // namespace Cube
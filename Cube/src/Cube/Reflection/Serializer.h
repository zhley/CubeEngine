#pragma once
#include "Any.h"
#include "ClassRegistry.h"
#include "Type.h"

#include <json.hpp>

namespace Cube {

    struct Converter {
        std::function<nlohmann::json(const Any& obj)> toJson;
        std::function<Any(const nlohmann::json&)> fromJson;
    };

    // depend on nlohmann::json
    class Serializer {
    public:
        static Serializer& get() {
            static Serializer instance;
            return instance;
        }

        void registerConverter(TypeID typeID, const Converter& converter) {
            registry[typeID] = converter;
        }

        nlohmann::json serialize(TypeID typeID, const Any& obj) {
            auto it = registry.find(typeID);
            if(it == registry.end()) {
                CB_CORE_ERROR("Serializer::serialize(): No converter registered for type ID {}", typeID);
                return nullptr;
            }
            return it->second.toJson(obj);
        }

        Any deserialize(TypeID typeID, const nlohmann::json& data) {
            auto it = registry.find(typeID);
            if(it == registry.end()) {
                CB_CORE_ERROR("Serializer::deserialize(): No converter registered for type ID {}", typeID);
                return Any();
            }
            return it->second.fromJson(data);
        }

        bool isRegistered(TypeID typeID) {
            return registry.find(typeID) != registry.end();
        }

    private:
        std::unordered_map<TypeID, Converter> registry;

        Serializer() = default;
        ~Serializer() = default;
    };

    // auto register serializers for basic types and classes
    template<typename T>
    void registerSerializer() {
        Converter converter;
        if constexpr (std::is_arithmetic_v<T> || std::is_same_v<T, std::string>) {
            converter.toJson = [](const Any& obj) {
                return nlohmann::json(obj.as<T>());
            };
            converter.fromJson = [](const nlohmann::json& data) {
                return Any(data.get<T>());
            };
        }else if constexpr (Type::SingleTraits<T>::container == Type::ContainerType::Vector) {
            converter.toJson = [](const Any& obj) {
                using ElementType = typename Type::SingleTraits<T>::Type;
                const T& vec = obj.as<T>();
                nlohmann::json j = nlohmann::json::array();
                for(const auto& element : vec) {
                    j.push_back(Serializer::get().serialize(getTypeID<ElementType>(), Any(element)));
                }
                return j;
            };
            converter.fromJson = [](const nlohmann::json& data) {
                using ElementType = typename Type::SingleTraits<T>::Type;
                T vec;
                for(const auto& elementData : data) {
                    Any element = Serializer::get().deserialize(getTypeID<ElementType>(), elementData);
                    vec.emplace(std::move(element.as<ElementType>()));
                }
                return Any(std::move(vec));
            };
        }else if constexpr (Type::MapTraits<T>::container == Type::ContainerType::UnorderedMap) {
            converter.toJson = [](const Any& obj) {
                using KeyType = typename Type::MapTraits<T>::KeyType;
                using ValueType = typename Type::MapTraits<T>::ValueType;
                const T& map = obj.as<T>();
                nlohmann::json j;
                for(const auto& [key, value] : map) {
                    j[Serializer::get().serialize(getTypeID<KeyType>(), Any(key)).get<std::string>()] =
                        Serializer::get().serialize(getTypeID<ValueType>(), Any(value));
                }
                return j;
            };
            converter.fromJson = [](const nlohmann::json& data) {
                using KeyType = typename Type::MapTraits<T>::KeyType;
                using ValueType = typename Type::MapTraits<T>::ValueType;
                T map;
                for(auto it = data.begin(); it != data.end(); ++it) {
                    Any keyAny = Serializer::get().deserialize(getTypeID<KeyType>(), nlohmann::json(it.key()));
                    Any valueAny = Serializer::get().deserialize(getTypeID<ValueType>(), it.value());
                    map.emplace(std::move(keyAny.as<KeyType>()), std::move(valueAny.as<ValueType>()));
                }
                return Any(std::move(map));
            };
        }else {
            static_assert(std::is_class_v<T>);
            converter.toJson = [](const Any& obj) {
                nlohmann::json j;
                Class* classInfo = ClassRegistry::get().getClass<T>();
                const void* data = nullptr;
                if(obj.getID() == getTypeID<T>()) {
                    data = obj.getData();
                } else if(removeAllPtr(obj.getID()) == classInfo->getTypeID() || removeAllPtr(obj.getID()) == classInfo->getBaseTypeID()) {
                    data = *((T**)(obj.getData()));  // TODO: Any启用小对象优化后这里需要修改
                } else {
                    CB_CORE_INFO("{}", getTypeID<T*>());
                    CB_ASSERT(0 && "Type mismatch!");
                }
                if(!classInfo) {
                    CB_CORE_ERROR("Serializer::toJson(): Class info not found for type ID {}", getTypeID<T>());
                    return j;
                }
                for(auto& property : classInfo->getAllProperties()) {
                    j[property->getName()] = Serializer::get().serialize(property->getTypeID(), property->getValue(data));
                }
                return j;
            };
            converter.fromJson = [](const nlohmann::json& data) {
                Class* classInfo = ClassRegistry::get().getClass<T>();
                if(!classInfo) {
                    CB_CORE_ERROR("Serializer::fromJson(): Class info not found for type ID {}", getTypeID<T>());
                    return Any();
                }
                Any instance = classInfo->createInstance();
                for(auto& property : classInfo->getAllProperties()) {
                    const nlohmann::json& propData = data[property->getName()];
                    Any propValue = Serializer::get().deserialize(property->getTypeID(), propData);
                    property->setValue(instance.getData(), std::move(propValue));
                }
                return instance;
            };
        }
        Serializer::get().registerConverter(getTypeID<T>(), converter);
    }

    inline void registerBasicSerializers() {
        registerSerializer<int8_t>();
        registerSerializer<uint8_t>();
        registerSerializer<int16_t>();
        registerSerializer<uint16_t>();
        registerSerializer<int32_t>();
        registerSerializer<uint32_t>();
        registerSerializer<int64_t>();
        registerSerializer<uint64_t>();
        registerSerializer<float>();
        registerSerializer<double>();
        registerSerializer<bool>();
        registerSerializer<std::string>();
    }
}
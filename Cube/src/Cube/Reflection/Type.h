#pragma once

#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

// TODO: 需要重新设计

namespace Cube {

    using TypeID = uint32_t;

    namespace Detail {
        inline TypeID nextArithmeticID; // ensure a unique copy
        inline TypeID nextClassID = 0x100;
    }

    namespace Type {
        template<typename T>
        constexpr bool isBarePtr() noexcept {
            using RawT = std::decay_t<T>;
            return std::is_pointer_v<RawT> && !std::is_member_pointer_v<RawT>;
        }

        template<typename T>
        struct PointerLevel {
            static constexpr uint8_t value = 0;
        };

        template<typename T>
        struct PointerLevel<T*> {
            static constexpr uint8_t value = 1 + PointerLevel<T>::value;
        };

        template<typename T>
        struct RemoveAllPointers {
            using Type = T;
        };

        template<typename T>
        struct RemoveAllPointers<T*> {
            using Type = typename RemoveAllPointers<T>::Type;
        };

        template<typename T>
        constexpr uint8_t getPtrLevel() noexcept {
            using RawT = std::decay_t<T>;
            static_assert(isBarePtr<RawT>());
            return PointerLevel<RawT>::value;
        }

        enum class ContainerType : uint8_t{
            Invalid,
            Vector,
            Set,
            UnorderedSet,
            Map,
            UnorderedMap,
            Pair
        };

        template<typename T>
        struct SingleTraits {
            using Type = void;
            static constexpr ContainerType container = ContainerType::Invalid;
        };

        template<typename T>
        struct SingleTraits<std::vector<T>> {
            using Type = T;
            static constexpr ContainerType container = ContainerType::Vector;
        };
        template<typename T>
        struct SingleTraits<std::set<T>> {
            using Type = T;
            static constexpr ContainerType container = ContainerType::Set;
        };
        template<typename T>
        struct SingleTraits<std::unordered_set<T>> {
            using Type = T;
            static constexpr ContainerType container = ContainerType::UnorderedSet;
        };

        template<typename T>
        struct MapTraits {
            using KeyType = void;
            using ValueType = void;
            static constexpr ContainerType container = ContainerType::Set;
        };

        template<typename Key, typename Value>
        struct MapTraits<std::map<Key, Value>> {
            using KeyType = Key;
            using ValueType = Value;
            static constexpr ContainerType container = ContainerType::Map;
        };
        template<typename Key, typename Value>
        struct MapTraits<std::unordered_map<Key, Value>> {
            using KeyType = Key;
            using ValueType = Value;
            static constexpr ContainerType container = ContainerType::UnorderedMap;
        };
        template<typename Key, typename Value>
        struct MapTraits<std::pair<Key, Value>> {
            using KeyType = Key;
            using ValueType = Value;
            static constexpr ContainerType container = ContainerType::Pair;
        };
    }

    namespace Detal {

        template<typename BareT>
        TypeID getBareTypeID() {
            if constexpr (Type::isBarePtr<BareT>()) {
                static_assert(Type::getPtrLevel<BareT>() < 8 && "Pointer depth is too deep!");
                return getBareTypeID<typename Type::RemoveAllPointers<BareT>::Type>() | (Type::getPtrLevel<BareT>() << 29);
            } else if constexpr (std::is_arithmetic_v<BareT>) {
                static TypeID id = Detail::nextArithmeticID++;
                return id;
            } else {
                static TypeID id = Detail::nextClassID++;
                return id;
            }
        }
    }

    template<typename T>
    TypeID getTypeID() {
        return Detal::getBareTypeID<std::decay_t<T>>();
    }

    inline bool isPtr(TypeID typeID) {
        return typeID >> 29;
    }

    inline TypeID removeAllPtr(TypeID typeID) {
        return typeID & ~(0x7u << 29);
    }

    inline bool isArithmetic(TypeID typeID) {
        return !(typeID >> 8);
    }
}
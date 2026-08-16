#pragma once

#include "BaseClassCaster.h"
#include "Cube/Core/Log.h"
#include "Type.h"

namespace Cube {

    // TODO: small object optimization
    class Any final{
    public:
        Any() : data(nullptr), typeID(0), destructor(nullptr){}
        Any(void* data, TypeID typeID, const std::function<void(void*)>& destructor)
            : data(data), typeID(typeID), destructor(destructor) {}

        // template<typename T, std::enable_if_t<!std::is_same_v<std::decay_t<T>, Any>, int> = 0>
        // Any(const T& value) : typeID(getTypeID<T>()) {
        //     if constexpr (std::is_copy_assignable_v<T>){
        //         data = new T(value);
        //         destructor = [](void* ptr){
        //             delete static_cast<T*>(ptr);
        //         };
        //     }else {
        //         CB_CORE_ERROR("Reflection: Failed to create Any instance for non-copy-assignable class");
        //         data = nullptr;
        //         typeID = 0;
        //         destructor = nullptr;
        //     }
        // }
        //
        // template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Any> && !std::is_lvalue_reference_v<T>>>
        // Any(T&& value) : typeID(getTypeID<T>()) {
        //     data = new T(std::forward<T>(value));
        //     destructor = [](void* ptr){
        //         delete static_cast<T*>(ptr);
        //     };
        // }

        template<typename T, typename = std::enable_if_t<!std::is_same_v<std::decay_t<T>, Any>>>
        Any(T&& value) : typeID(getTypeID<std::decay_t<T>>()) {
            using RawT = std::decay_t<T>;
            data = new RawT(std::forward<T>(value));
            destructor = [](void* ptr){
                delete static_cast<RawT*>(ptr);
            };
        }

        // as an observer, does not own the pointer
        // template<typename T>
        // Any(T* ptr) : typeID(getTypeID<T>()), destructor(nullptr) {
        //     data = ptr;
        // }

        Any(const Any& other) = delete;
        Any& operator=(const Any& other) = delete;

        Any(Any&& other) noexcept : data(other.data), typeID(other.typeID), destructor(std::move(other.destructor)){
            other.data = nullptr;
            other.typeID = 0;
            other.destructor = nullptr;
        }

        ~Any() {
            if(data) {
                if(destructor){
                    destructor(data);
                }
            }
        }

        Any& operator=(Any&& other) noexcept {
            if(this != &other) {
                data = other.data;
                typeID = other.typeID;
                destructor = other.destructor;
                other.data = nullptr;
                other.typeID = 0;
                other.destructor = nullptr;
            }
            return *this;
        }

        template<typename T>
        T& as() {
            if(typeID != getTypeID<T>()) {
                CB_CORE_ERROR("Reflection: Type mismatched in Any::as()");
                CB_ASSERT(0); // we assume this will never happen in release build
            }
            return *static_cast<T*>(data);
        }

        template<typename T>
        const T& as() const{
            if(typeID != getTypeID<T>()) {
                CB_CORE_ERROR("Reflection: Type mismatched in Any::as()");
                CB_ASSERT(0); // we assume this will never happen in release build
            }
            return *static_cast<const T*>(data);
        }

        // transfer the ownership of the internal data pointer. 
        // type must match T strictly. if you only know the base class type, use moveToBase() instead.
        template<typename T>
        T* move() {
            CB_ASSERT(typeID == getTypeID<T>());
            T* ptr = static_cast<T*>(data);
            data = nullptr;
            typeID = 0;
            destructor = nullptr;
            return ptr;
        }

        // transfer the ownership of the internal data pointer and cast to base class.
        template<typename BaseType>
        BaseType* moveToBase() {
            BaseType* bt = BaseClassCaster<BaseType>::cast(typeID, data);
            data = nullptr;
            typeID = 0;
            destructor = nullptr;
            return bt;
        }

        TypeID getID() const { return typeID; }
        void* getData(){ return data; }
        const void* getData() const { return data; }
        bool isNull() const { return data == nullptr; }

    private:
        void* data;
        TypeID typeID;
        std::function<void(void*)> destructor;
    };

}
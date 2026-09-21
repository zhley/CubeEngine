#pragma once

#include <memory>
#include <unordered_map>

#include "zeta/vm/value.h"
#include "zeta/vm/vm.h"

#include "Cube/Reflection/Type.h"

namespace Cube {

class Class;
class Component;
class Node;

// Host NativeType for engine Node. Only public data members are properties.
class NodeType final : public Zeta::NativeType {
public:
    explicit NodeType(Zeta::VM* vm);

    void getField(void* instance, Zeta::String* fieldName) override;
    void setField(void* instance, Zeta::String* fieldName, const Zeta::Value& value) override;
    void callMethod(void* instance, Zeta::String* methodName, int argc) override;

private:
    Zeta::String* namePos = nullptr;
    Zeta::String* nameRotation = nullptr;
    Zeta::String* nameScale = nullptr;
    Zeta::String* nameEquals = nullptr;
    Zeta::String* nameGetName = nullptr;
    Zeta::String* nameGetParent = nullptr;
    Zeta::String* nameFindChild = nullptr;
    Zeta::String* nameAddChild = nullptr;
    Zeta::String* nameRemoveChild = nullptr;
    Zeta::String* nameHasComponent = nullptr;
    Zeta::String* nameGetComponent = nullptr;
    Zeta::String* nameAddComponent = nullptr;
    Zeta::String* nameRemoveComponent = nullptr;
};

// Per reflected-component-class NativeType. Different component types are
// different UserData instances that share this C++ base but not this pointer.
// Fields/methods forward to Cube reflection Class.
class ComponentType final : public Zeta::NativeType {
public:
    ComponentType(Zeta::VM* vm, Class* classInfo);

    void getField(void* instance, Zeta::String* fieldName) override;
    void setField(void* instance, Zeta::String* fieldName, const Zeta::Value& value) override;
    void callMethod(void* instance, Zeta::String* methodName, int argc) override;

private:
    Class* classInfo = nullptr;
    Zeta::String* nameEquals = nullptr;
};

class ScriptBindings {
public:
    static void initialize(Zeta::VM& vm);
    static void shutdown();

    static void wrapNode(Zeta::VM& vm, Node* node);
    static void wrapComponent(Zeta::VM& vm, Component* component);

    // cube module global indices for value types
    static int vec2Class;
    static int vec3Class;
    static int vec4Class;
    static int colorClass;

private:
    static Zeta::VM* vm;
    static std::unique_ptr<NodeType> nodeType;
    static std::unordered_map<TypeID, std::unique_ptr<ComponentType>> componentTypes;

    static ComponentType* getComponentType(Class* classInfo);
};

}  // namespace Cube

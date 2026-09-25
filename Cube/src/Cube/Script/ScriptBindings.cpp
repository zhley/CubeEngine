#include "ScriptBindings.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "glm/glm.hpp"
#include "json.hpp"
#include "zeta/compiler/bytecode.h"

#include "Cube/Animation/AnimationClip.h"
#include "Cube/Core/Engine.h"
#include "Cube/Core/Input.h"
#include "Cube/Reflection/Any.h"
#include "Cube/Reflection/Class.h"
#include "Cube/Reflection/ClassRegistry.h"
#include "Cube/Reflection/Type.h"
#include "Cube/Renderer/Color.h"
#include "Cube/Renderer/Texture.h"
#include "Cube/Renderer/Font.h"
#include "Cube/Resource/ResPtr.h"
#include "Cube/Resource/Script.h"
#include "Cube/Resource/Sprite.h"
#include "Cube/Scene/Component.h"
#include "Cube/Scene/Node.h"
#include "Cube/Script/ScriptRuntime.h"

namespace Cube {

Zeta::VM* ScriptBindings::vm = nullptr;
std::unique_ptr<NodeType> ScriptBindings::nodeType;
std::unordered_map<TypeID, std::unique_ptr<ComponentType>> ScriptBindings::componentTypes;
int ScriptBindings::vec2Class = -1;
int ScriptBindings::vec3Class = -1;
int ScriptBindings::vec4Class = -1;
int ScriptBindings::colorClass = -1;

namespace {

std::string toString(const Zeta::String* str) {
    return std::string(str->getData(), str->getLength());
}

void pushNull(Zeta::VM* vm) {
    vm->push(Zeta::Value::Null);
}

void pushError(Zeta::VM* vm, const std::string& message) {
    vm->reportError(message);
    vm->push(Zeta::Value::Error);
}

void pushVec2(Zeta::VM* vm, const glm::vec2& v) {
    vm->push(Zeta::Value(static_cast<double>(v.x)));
    vm->push(Zeta::Value(static_cast<double>(v.y)));
    vm->push(vm->getGlobal(ScriptBindings::vec2Class));
    vm->newInstance(2);
}

void pushVec3(Zeta::VM* vm, const glm::vec3& v) {
    vm->push(Zeta::Value(static_cast<double>(v.x)));
    vm->push(Zeta::Value(static_cast<double>(v.y)));
    vm->push(Zeta::Value(static_cast<double>(v.z)));
    vm->push(vm->getGlobal(ScriptBindings::vec3Class));
    vm->newInstance(3);
}

void pushVec4(Zeta::VM* vm, const glm::vec4& v) {
    vm->push(Zeta::Value(static_cast<double>(v.x)));
    vm->push(Zeta::Value(static_cast<double>(v.y)));
    vm->push(Zeta::Value(static_cast<double>(v.z)));
    vm->push(Zeta::Value(static_cast<double>(v.w)));
    vm->push(vm->getGlobal(ScriptBindings::vec4Class));
    vm->newInstance(4);
}

void pushColor(Zeta::VM* vm, const Color& c) {
    vm->push(Zeta::Value(static_cast<double>(c.r)));
    vm->push(Zeta::Value(static_cast<double>(c.g)));
    vm->push(Zeta::Value(static_cast<double>(c.b)));
    vm->push(Zeta::Value(static_cast<double>(c.a)));
    vm->push(vm->getGlobal(ScriptBindings::colorClass));
    vm->newInstance(4);
}

std::optional<glm::vec2> toVec2(Zeta::VM* vm, const Zeta::Value& value) {
    auto xVal = Zeta::Value(value[vm->internString("x")]).as<float>();
    auto yVal = Zeta::Value(value[vm->internString("y")]).as<float>();
    if (xVal.has_value() && yVal.has_value()) {
        return glm::vec2(*xVal, *yVal);
    }
    return std::nullopt;
}

std::optional<glm::vec3> toVec3(Zeta::VM* vm, const Zeta::Value& value) {
    auto xVal = Zeta::Value(value[vm->internString("x")]).as<float>();
    auto yVal = Zeta::Value(value[vm->internString("y")]).as<float>();
    auto zVal = Zeta::Value(value[vm->internString("z")]).as<float>();
    if (xVal.has_value() && yVal.has_value() && zVal.has_value()) {
        return glm::vec3(*xVal, *yVal, *zVal);
    }
    return std::nullopt;
}

std::optional<glm::vec4> toVec4(Zeta::VM* vm, const Zeta::Value& value) {
    auto xVal = Zeta::Value(value[vm->internString("x")]).as<float>();
    auto yVal = Zeta::Value(value[vm->internString("y")]).as<float>();
    auto zVal = Zeta::Value(value[vm->internString("z")]).as<float>();
    auto wVal = Zeta::Value(value[vm->internString("w")]).as<float>();
    if (xVal.has_value() && yVal.has_value() && zVal.has_value() && wVal.has_value()) {
        return glm::vec4(*xVal, *yVal, *zVal, *wVal);
    }
    return std::nullopt;
}

std::optional<Color> toColor(Zeta::VM* vm, const Zeta::Value& value) {
    auto rVal = Zeta::Value(value[vm->internString("r")]).as<float>();
    auto gVal = Zeta::Value(value[vm->internString("g")]).as<float>();
    auto bVal = Zeta::Value(value[vm->internString("b")]).as<float>();
    auto aVal = Zeta::Value(value[vm->internString("a")]).as<float>();
    if (rVal.has_value() && gVal.has_value() && bVal.has_value() && aVal.has_value()) {
        return Color(*rVal, *gVal, *bVal, *aVal);
    }
    return std::nullopt;
}

void pushAny(Zeta::VM* vm, const Any& value) {
    if (value.isNull()) {
        pushNull(vm);
        return;
    }
    TypeID typeID = value.getID();
    if (typeID == getTypeID<int>()) {
        vm->push(Zeta::Value(static_cast<int64_t>(value.as<int>())));
        return;
    }
    if (typeID == getTypeID<float>()) {
        vm->push(Zeta::Value(static_cast<double>(value.as<float>())));
        return;
    }
    if (typeID == getTypeID<double>()) {
        vm->push(Zeta::Value(value.as<double>()));
        return;
    }
    if (typeID == getTypeID<bool>()) {
        vm->push(Zeta::Value(value.as<bool>()));
        return;
    }
    if (typeID == getTypeID<std::string>()) {
        vm->newStrObj(value.as<std::string>());
        return;
    }
    if (typeID == getTypeID<glm::vec2>()) {
        pushVec2(vm, value.as<glm::vec2>());
        return;
    }
    if (typeID == getTypeID<glm::vec3>()) {
        pushVec3(vm, value.as<glm::vec3>());
        return;
    }
    if (typeID == getTypeID<glm::vec4>()) {
        pushVec4(vm, value.as<glm::vec4>());
        return;
    }
    if (typeID == getTypeID<Color>()) {
        pushColor(vm, value.as<Color>());
        return;
    }
    if (typeID == getTypeID<ResPtr<Sprite>>()) {
        const ResPtr<Sprite>& res = value.as<ResPtr<Sprite>>();
        if (res.isNull()) {
            pushNull(vm);
        } else {
            vm->newStrObj(res->getIdentifier());
        }
        return;
    }
    if (typeID == getTypeID<ResPtr<Texture2D>>()) {
        const ResPtr<Texture2D>& res = value.as<ResPtr<Texture2D>>();
        if (res.isNull()) {
            pushNull(vm);
        } else {
            vm->newStrObj(res->getIdentifier());
        }
        return;
    }
    if (typeID == getTypeID<ResPtr<AnimationClip>>()) {
        const ResPtr<AnimationClip>& res = value.as<ResPtr<AnimationClip>>();
        if (res.isNull()) {
            pushNull(vm);
        } else {
            vm->newStrObj(res->getIdentifier());
        }
        return;
    }
    if (typeID == getTypeID<ResPtr<Font>>()) {
        const ResPtr<Font>& res = value.as<ResPtr<Font>>();
        if (res.isNull()) {
            pushNull(vm);
        } else {
            vm->newStrObj(res->getIdentifier());
        }
        return;
    }
    if (typeID == getTypeID<ResPtr<Script>>()) {
        const ResPtr<Script>& res = value.as<ResPtr<Script>>();
        if (res.isNull()) {
            pushNull(vm);
        } else {
            vm->newStrObj(res->getIdentifier());
        }
        return;
    }
    // TODO: 支持更多类型
    pushNull(vm);
}

Any toAny(Zeta::VM* vm, TypeID typeID, const Zeta::Value& value) {
    if (typeID == getTypeID<int>()) {
        auto val = value.as<int>();
        return val.has_value() ? Any(*val) : Any();
    }
    if (typeID == getTypeID<float>()) {
        auto val = value.as<float>();
        return val.has_value() ? Any(*val) : Any();
    }
    if (typeID == getTypeID<double>()) {
        auto val = value.as<double>();
        return val.has_value() ? Any(*val) : Any();
    }
    if (typeID == getTypeID<bool>()) {
        auto val = value.as<bool>();
        return val.has_value() ? Any(*val) : Any();
    }
    if (typeID == getTypeID<std::string>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(*val) : Any();
    }
    if (typeID == getTypeID<glm::vec2>()) {
        auto val = toVec2(vm, value);
        return val.has_value() ? Any(*val) : Any();
    }
    if (typeID == getTypeID<glm::vec3>()) {
        auto val = toVec3(vm, value);
        return val.has_value() ? Any(*val) : Any();
    }
    if (typeID == getTypeID<glm::vec4>()) {
        auto val = toVec4(vm, value);
        return val.has_value() ? Any(*val) : Any();
    }
    if (typeID == getTypeID<Color>()) {
        auto val = toColor(vm, value);
        return val.has_value() ? Any(*val) : Any();
    }
    if (typeID == getTypeID<ResPtr<Sprite>>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(ResPtr<Sprite>(*val)) : Any();
    }
    if (typeID == getTypeID<ResPtr<Texture2D>>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(ResPtr<Texture2D>(*val)) : Any();
    }
    if (typeID == getTypeID<ResPtr<AnimationClip>>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(ResPtr<AnimationClip>(*val)) : Any();
    }
    if (typeID == getTypeID<ResPtr<Font>>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(ResPtr<Font>(*val)) : Any();
    }
    if (typeID == getTypeID<ResPtr<Script>>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(ResPtr<Script>(*val)) : Any();
    }
    return Any();
}

void wrapUserData(Zeta::VM* vm, void* ptr, Zeta::NativeType* type) {
    if (!ptr || !type) {
        pushNull(vm);
        return;
    }
    vm->newUserData(ptr, type);
}

void cbGetRoot(Zeta::VM* vm, int argc) {
    if (argc != 0) {
        pushError(vm, "cb_get_root: argument mismatch");
        return;
    }
    Application* app = Engine::getApp();
    if (!app) {
        pushNull(vm);
        return;
    }
    ScriptBindings::wrapNode(*vm, app->getRootNode());
}

void cbIsKeyPressed(Zeta::VM* vm, int argc) {
    if (argc != 1) {
        pushError(vm, "cb_is_key_pressed: argument mismatch");
        return;
    }
    auto key = vm->getLocal(0).as<int>();
    if (!key.has_value()) {
        pushError(vm, "cb_is_key_pressed: key must be int");
        return;
    }
    vm->push(Zeta::Value(Input::isKeyPressed(static_cast<KeyCode>(*key))));
}

void cbIsMouseButtonPressed(Zeta::VM* vm, int argc) {
    if (argc != 1) {
        pushError(vm, "cb_is_mouse_button_pressed: argument mismatch");
        return;
    }
    auto button = vm->getLocal(0).as<int>();
    if (!button.has_value()) {
        pushError(vm, "cb_is_mouse_button_pressed: button must be int");
        return;
    }
    vm->push(Zeta::Value(Input::isMouseButtonPressed(static_cast<MouseCode>(*button))));
}

void cbGetMousePos(Zeta::VM* vm, int argc) {
    if (argc != 0) {
        pushError(vm, "cb_get_mouse_pos: argument mismatch");
        return;
    }
    pushVec2(vm, Input::getMousePos());
}

bool equalsUserData(const Zeta::Value& other, const Zeta::NativeType* type, const void* hostPtr) {
    if (!other.isUserData()) {
        return false;
    }
    auto* ud = static_cast<Zeta::UserData*>(other.ptrValue);
    return ud->getNativeType() == type && ud->getData() == hostPtr;
}

}  // namespace

NodeType::NodeType(Zeta::VM* vm) : Zeta::NativeType(vm) {
    namePos = vm->internString("pos");
    nameRotation = vm->internString("rotation");
    nameScale = vm->internString("scale");
    nameEquals = vm->internString("_equals");
    nameGetName = vm->internString("get_name");
    nameGetParent = vm->internString("get_parent");
    nameFindChild = vm->internString("find_child");
    nameAddChild = vm->internString("add_child");
    nameRemoveChild = vm->internString("remove_child");
    nameHasComponent = vm->internString("has_component");
    nameGetComponent = vm->internString("get_component");
    nameAddComponent = vm->internString("add_component");
    nameRemoveComponent = vm->internString("remove_component");
}

void NodeType::getField(void* instance, Zeta::String* fieldName) {
    auto* node = static_cast<Node*>(instance);
    if (!node) {
        pushNull(vm);
        return;
    }
    if (fieldName == namePos) {
        pushVec2(vm, node->pos);
        return;
    }
    if (fieldName == nameRotation) {
        vm->push(Zeta::Value(static_cast<double>(node->rotation)));
        return;
    }
    if (fieldName == nameScale) {
        pushVec2(vm, node->scale);
        return;
    }
    pushError(vm, "Node.get_field: unknown field '" + toString(fieldName) + "'");
}

void NodeType::setField(void* instance, Zeta::String* fieldName, const Zeta::Value& value) {
    auto* node = static_cast<Node*>(instance);
    if (!node) {
        vm->reportError("Node.set_field: node is null");
        return;
    }
    if (fieldName == namePos) {
        auto pos = toVec2(vm, value);
        if (!pos.has_value()) {
            vm->reportError("Node.pos: expected Vec2-like value");
            return;
        }
        node->pos = *pos;
        return;
    }
    if (fieldName == nameRotation) {
        auto rotation = value.as<float>();
        if (!rotation.has_value()) {
            vm->reportError("Node.rotation: expected number");
            return;
        }
        node->rotation = *rotation;
        return;
    }
    if (fieldName == nameScale) {
        auto scale = toVec2(vm, value);
        if (!scale.has_value()) {
            vm->reportError("Node.scale: expected Vec2-like value");
            return;
        }
        node->scale = *scale;
        return;
    }
    vm->reportError("Node.set_field: unknown field '" + toString(fieldName) + "'");
}

void NodeType::callMethod(void* instance, Zeta::String* methodName, int argc) {
    auto* node = static_cast<Node*>(instance);
    if (!node) {
        pushError(vm, "Node.call_method: node is null");
        return;
    }
    if (methodName == nameEquals) {
        if (argc != 2) {
            pushError(vm, "Node._equals: argc must be 2");
            return;
        }
        vm->push(Zeta::Value(equalsUserData(vm->getLocal(1), this, instance)));
        return;
    }
    if (methodName == nameGetName) {
        if (argc != 1) {
            pushError(vm, "Node.get_name: argc must be 1");
            return;
        }
        vm->newStrObj(node->getName());
        return;
    }
    if (methodName == nameGetParent) {
        if (argc != 1) {
            pushError(vm, "Node.get_parent: argc must be 1");
            return;
        }
        wrapUserData(vm, node->getParent(), this);
        return;
    }
    if (methodName == nameFindChild) {
        if (argc != 2) {
            pushError(vm, "Node.find_child: argc must be 2");
            return;
        }
        auto name = vm->getLocal(1).as<std::string>();
        if (!name.has_value()) {
            pushError(vm, "Node.find_child: name must be string");
            return;
        }
        wrapUserData(vm, node->findChild(*name), this);
        return;
    }
    if (methodName == nameAddChild) {
        if (argc != 2) {
            pushError(vm, "Node.add_child: argc must be 2");
            return;
        }
        auto name = vm->getLocal(1).as<std::string>();
        if (!name.has_value()) {
            pushError(vm, "Node.add_child: name must be string");
            return;
        }
        wrapUserData(vm, node->addChild(*name), this);
        return;
    }
    if (methodName == nameRemoveChild) {
        if (argc != 2) {
            pushError(vm, "Node.remove_child: argc must be 2");
            return;
        }
        auto name = vm->getLocal(1).as<std::string>();
        if (!name.has_value()) {
            pushError(vm, "Node.remove_child: name must be string");
            return;
        }
        node->removeChild(*name);
        pushNull(vm);
        return;
    }
    if (methodName == nameHasComponent) {
        if (argc != 2) {
            pushError(vm, "Node.has_component: argc must be 2");
            return;
        }
        auto typeName = vm->getLocal(1).as<std::string>();
        if (!typeName.has_value()) {
            pushError(vm, "Node.has_component: type must be string");
            return;
        }
        vm->push(Zeta::Value(node->hasComponent(*typeName)));
        return;
    }
    if (methodName == nameGetComponent) {
        if (argc != 2) {
            pushError(vm, "Node.get_component: argc must be 2");
            return;
        }
        auto typeName = vm->getLocal(1).as<std::string>();
        if (!typeName.has_value()) {
            pushError(vm, "Node.get_component: type must be string");
            return;
        }
        Component* component = node->getComponent(*typeName);
        if (!component) {
            pushNull(vm);
            return;
        }
        if (component->getType() == getTypeID<ScriptComponent>()) {
            // For ScriptComponent, return the original Zeta instance
            ScriptComponent* scriptComp = static_cast<ScriptComponent*>(component);
            vm->push(scriptComp->getInstance() ? *scriptComp->getInstance() : Zeta::Value::Null);
        } else {
            ScriptBindings::wrapComponent(*vm, component);
        }
        return;
    }
    if (methodName == nameAddComponent) {
        if (argc == 2) {
            auto typeName = vm->getLocal(1).as<std::string>();
            if (!typeName.has_value()) {
                pushError(vm, "Node.add_component: type must be string");
                return;
            }
            ScriptBindings::wrapComponent(*vm, node->addComponent(*typeName));
            return;
        } else if (argc == 3) {
            auto scriptIdentifier = vm->getLocal(1).as<std::string>();
            auto scriptComponentName = vm->getLocal(2).as<std::string>();
            if (!scriptIdentifier.has_value() || !scriptComponentName.has_value()) {
                pushError(vm, "Node.add_component: scriptIdentifier and scriptComponentName must be strings");
                return;
            }
            ScriptComponent* scriptComp = node->addComponent<ScriptComponent>(*scriptIdentifier, *scriptComponentName);
            if (!scriptComp || !scriptComp->getInstance()) {
                pushError(vm, "Node.add_component: failed to add ScriptComponent");
                return;
            }
            vm->push(*scriptComp->getInstance());
            return;
        } else {
            pushError(vm, "Node.add_component: argc must be 2 or 3");
            return;
        }
    }
    if (methodName == nameRemoveComponent) {
        if (argc != 2) {
            pushError(vm, "Node.remove_component: argc must be 2");
            return;
        }
        auto typeName = vm->getLocal(1).as<std::string>();
        if (!typeName.has_value()) {
            pushError(vm, "Node.remove_component: type must be string");
            return;
        }
        node->removeComponent(*typeName);
        pushNull(vm);
        return;
    }
    pushError(vm, "Node.call_method: unknown method '" + toString(methodName) + "'");
}

ComponentType::ComponentType(Zeta::VM* vm, Class* classInfo)
    : Zeta::NativeType(vm), classInfo(classInfo) {
    nameEquals = vm->internString("_equals");
}

void ComponentType::getField(void* instance, Zeta::String* fieldName) {
    if (!instance || !classInfo) {
        pushNull(vm);
        return;
    }
    Property* property = classInfo->getProperty(toString(fieldName));
    if (!property) {
        pushError(vm, "Component.get_field: unknown property '" + toString(fieldName) + "'");
        return;
    }
    pushAny(vm, property->getValue(instance));
}

void ComponentType::setField(void* instance, Zeta::String* fieldName, const Zeta::Value& value) {
    if (!instance || !classInfo) {
        vm->reportError("Component.set_field: instance is null");
        return;
    }
    Property* property = classInfo->getProperty(toString(fieldName));
    if (!property) {
        vm->reportError("Component.set_field: unknown property '" + toString(fieldName) + "'");
        return;
    }
    Any anyValue = toAny(vm, property->getTypeID(), value);
    if (anyValue.isNull()) {
        vm->reportError("Component.set_field: value type mismatch for '" + toString(fieldName) + "'");
        return;
    }
    property->setValue(instance, anyValue);
}

void ComponentType::callMethod(void* instance, Zeta::String* methodName, int argc) {
    if (!instance || !classInfo) {
        pushError(vm, "Component.call_method: instance is null");
        return;
    }
    if (methodName == nameEquals) {
        if (argc != 2) {
            pushError(vm, "Component._equals: argc must be 2");
            return;
        }
        vm->push(Zeta::Value(equalsUserData(vm->getLocal(1), this, instance)));
        return;
    }
    Method* method = classInfo->getMethod(toString(methodName));
    if (!method) {
        pushError(vm, "Component.call_method: unknown method '" + toString(methodName) + "'");
        return;
    }
    const int argCount = argc - 1;
    const std::vector<TypeID>& params = method->getParameters();
    if (static_cast<int>(params.size()) != argCount) {
        pushError(vm, "Component.call_method: argument count mismatch for '" + toString(methodName) + "'");
        return;
    }
    std::vector<Any> args(static_cast<size_t>(argCount));
    for (int i = 0; i < argCount; ++i) {
        args[static_cast<size_t>(i)] = toAny(vm, params[static_cast<size_t>(i)], vm->getLocal(static_cast<uint32_t>(1 + i)));
        if (args[static_cast<size_t>(i)].isNull()) {
            pushError(vm, "Component.call_method: argument type mismatch for '" + toString(methodName) + "'");
            return;
        }
    }
    pushAny(vm, method->invoke(instance, args));
}

void ScriptBindings::initialize(Zeta::VM& runtimeVm) {
    vm = &runtimeVm;
    nodeType = std::make_unique<NodeType>(vm);

    vm->registerFunction("cb_get_root", cbGetRoot);
    vm->registerFunction("cb_is_key_pressed", cbIsKeyPressed);
    vm->registerFunction("cb_is_mouse_button_pressed", cbIsMouseButtonPressed);
    vm->registerFunction("cb_get_mouse_pos", cbGetMousePos);

    auto cubeCoreScriptPath = vm->searchModuleFile("cube").first.string();
    std::unique_ptr<Zeta::Module> coreModule = ScriptRuntime::parseModule(Cube::Path(cubeCoreScriptPath));
    if (!coreModule) {
        return;
    }
    vm->loadModule(coreModule.get());
    vec2Class = vm->findGlobal(coreModule->name, "Vec2");
    vec3Class = vm->findGlobal(coreModule->name, "Vec3");
    vec4Class = vm->findGlobal(coreModule->name, "Vec4");
    colorClass = vm->findGlobal(coreModule->name, "Color");
}

void ScriptBindings::shutdown() {
    componentTypes.clear();
    nodeType.reset();
    vm = nullptr;
    vec2Class = -1;
    vec3Class = -1;
    vec4Class = -1;
    colorClass = -1;
}

void ScriptBindings::wrapNode(Zeta::VM& runtimeVm, Node* node) {
    wrapUserData(&runtimeVm, node, nodeType.get());
}

void ScriptBindings::wrapComponent(Zeta::VM& runtimeVm, Component* component) {
    if (!component) {
        pushNull(&runtimeVm);
        return;
    }
    Class* classInfo = ClassRegistry::get().getClass(component->getType());
    if (!classInfo) {
        pushNull(&runtimeVm);
        return;
    }
    ComponentType* type = getComponentType(classInfo);
    wrapUserData(&runtimeVm, component, type);
}

nlohmann::json ScriptBindings::serializeBasicValue(const Zeta::Value& value) {
    nlohmann::json data;
    switch (value.type) {
        case Zeta::Value::Type::Null:
            data["type"] = "Null";
            data["value"] = nullptr;
            break;
        case Zeta::Value::Type::Int:
            data["type"] = "Int";
            data["value"] = *value.as<int64_t>();
            break;
        case Zeta::Value::Type::Float:
            data["type"] = "Float";
            data["value"] = *value.as<double>();
            break;
        case Zeta::Value::Type::Bool:
            data["type"] = "Bool";
            data["value"] = *value.as<bool>();
            break;
        case Zeta::Value::Type::String:
            data["type"] = "String";
            data["value"] = *value.as<std::string>();
            break;
        case Zeta::Value::Type::Object:{
            Zeta::Object* obj = static_cast<Zeta::Object*>(value.ptrValue);
            switch (obj->getType()) {
                case Zeta::Object::Type::StrObj: {
                    data["type"] = "StrObj";
                    data["value"] = *value.as<std::string>();
                    break;
                }
                case Zeta::Object::Type::Array: {
                    data["type"] = "Array";
                    nlohmann::json arrayData = nlohmann::json::array();
                    Zeta::Array* arr = static_cast<Zeta::Array*>(obj);
                    arr->forEach([&arrayData](const Zeta::Value& elem) {
                        arrayData.push_back(serializeBasicValue(elem));
                    });
                    data["value"] = arrayData;
                    break;
                }
                case Zeta::Object::Type::Map: {
                    data["type"] = "Map";
                    nlohmann::json mapData;
                    Zeta::Map* map = static_cast<Zeta::Map*>(obj);
                    map->forEach([&mapData](const Zeta::String* key, const Zeta::Value& val) {
                        mapData[std::string(key->getData(), key->getLength())] = serializeBasicValue(val);
                    });
                    data["value"] = mapData;
                    break;
                }
                default:
                    data["type"] = "unsupported";
                    break;
            }
        }
        default:
            data["type"] = "unsupported";
            break;
    }
    return data;
}

Zeta::Value ScriptBindings::deserializeBasicValue(const nlohmann::json& data) {
    if (!data.contains("type") || !data.contains("value")) {
        return Zeta::Value::Null;
    }
    std::string type = data["type"];
    if (type == "Null") {
        return Zeta::Value::Null;
    } else if (type == "Int") {
        return Zeta::Value(data["value"].get<int64_t>());
    } else if (type == "Float") {
        return Zeta::Value(data["value"].get<double>());
    } else if (type == "Bool") {
        return Zeta::Value(data["value"].get<bool>());
    } else if (type == "String") {
        return Zeta::Value(vm->internString(data["value"].get<std::string>()));
    } else if (type == "Array") {
        const auto& arrayData = data["value"];
        vm->newArray(arrayData.size());
        for (int i = 0; i < arrayData.size(); ++i) {
            (*vm->peek(-1)->as<Zeta::Array*>())->set(i, deserializeBasicValue(arrayData[i]));
        }
        return vm->pop();
    } else if (type == "Map") {
        const auto& mapData = data["value"];
        vm->newMap();
        for (auto& [key, val] : mapData.items()) {
            (*vm->peek(-1)->as<Zeta::Map*>())->set(vm->internString(key), deserializeBasicValue(val));
        }
        return vm->pop();
    } else {
        return Zeta::Value::Null;
    }
}

ComponentType* ScriptBindings::getComponentType(Class* classInfo) {
    if (!classInfo || !vm) {
        return nullptr;
    }
    TypeID typeID = classInfo->getTypeID();
    auto it = componentTypes.find(typeID);
    if (it != componentTypes.end()) {
        return it->second.get();
    }
    auto type = std::make_unique<ComponentType>(vm, classInfo);
    ComponentType* raw = type.get();
    componentTypes[typeID] = std::move(type);
    return raw;
}

}  // namespace Cube

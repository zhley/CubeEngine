#include "ScriptBindings.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>

#include "glm/glm.hpp"
#include "zeta/compiler/bytecode.h"

#include "Cube/Reflection/Type.h"
#include "Cube/Core/Input.h"
#include "Cube/Reflection/ClassRegistry.h"
#include "Cube/Scene/Component.h"
#include "Cube/Scene/Entity.h"
#include "Cube/Scene/Scene.h"
#include "Cube/Scene/SceneManager.h"
#include "Cube/Scene/Transform.h"
#include "Cube/Script/ScriptRuntime.h"
#include "Cube/Renderer/Color.h"
#include "Cube/Resource/ResPtr.h"
#include "Cube/Resource/Sprite.h"
#include "Cube/Renderer/Texture.h"
#include "Cube/Animation/AnimationClip.h"
#include "Cube/Renderer/Font.h"
#include "Cube/Resource/Script.h"

namespace Cube {

int ScriptBindings::entityClass = -1;
int ScriptBindings::transformClass = -1;
int ScriptBindings::componentClass = -1;
int ScriptBindings::sceneClass = -1;
int ScriptBindings::vec2Class = -1;
int ScriptBindings::vec3Class = -1;
int ScriptBindings::vec4Class = -1;

namespace {

// NOTE: Zeta 原生函数与宿主通过 VM 操作数栈交换数据:
// - 方法调用时, 实例对象位于栈顶(peek(-1)), argc = 形参个数 + 1, 实际参数在实例下方倒序排列;
// - 全局函数调用时, 参数位于栈顶, argc = 实参个数;
// - 原生函数必须弹出全部 argc 个参数, 再压入一个返回值.

void wrapPtr(Zeta::VM* vm, void* ptr, int classIndex) {
    if(!ptr) {
        vm->push(Zeta::Value::Null);
        return;
    }
    vm->wrapPointer(ptr, vm->getGlobal(classIndex));
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
    vm->push(vm->getGlobal(ScriptBindings::vec4Class));
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

// 把反射 Any 值转换为 Zeta 值并压入栈
void pushAny(Zeta::VM* vm, const Any& value) {
    TypeID typeID = value.getID();
    if(typeID == getTypeID<int>()) {
        vm->push(Zeta::Value(static_cast<int64_t>(value.as<int>())));
        return;
    }
    if(typeID == getTypeID<float>()) {
        vm->push(Zeta::Value(static_cast<double>(value.as<float>())));
        return;
    }
    if(typeID == getTypeID<double>()) {
        vm->push(Zeta::Value(value.as<double>()));
        return;
    }
    if(typeID == getTypeID<bool>()) {
        vm->push(Zeta::Value(value.as<bool>()));
        return;
    }
    if(typeID == getTypeID<std::string>()) {
        vm->newStrObj(value.as<std::string>());
        return;
    }
    if(typeID == getTypeID<glm::vec2>()) {
        pushVec2(vm, value.as<glm::vec2>());
        return;
    }
    if(typeID == getTypeID<glm::vec3>()) {
        pushVec3(vm, value.as<glm::vec3>());
        return;
    }
    if(typeID == getTypeID<glm::vec4>()) {
        pushVec4(vm, value.as<glm::vec4>());
        return;
    }
    if(typeID == getTypeID<Color>()) {
        pushColor(vm, value.as<Color>());
        return;
    }
    if(typeID == getTypeID<ResPtr<Sprite>>()) {
        vm->newStrObj(value.as<ResPtr<Sprite>>()->getIdentifier());
        return;
    }
    if(typeID == getTypeID<ResPtr<Texture2D>>()) {
        vm->newStrObj(value.as<ResPtr<Texture2D>>()->getIdentifier());
        return;
    }
    if(typeID == getTypeID<ResPtr<AnimationClip>>()) {
        vm->newStrObj(value.as<ResPtr<AnimationClip>>()->getIdentifier());
        return;
    }
    if(typeID == getTypeID<ResPtr<Font>>()) {
        vm->newStrObj(value.as<ResPtr<Font>>()->getIdentifier());
        return;
    }
    if(typeID == getTypeID<ResPtr<Script>>()) {
        vm->newStrObj(value.as<ResPtr<Script>>()->getIdentifier());
        return;
    }
    // TODO: 支持更多类型
    vm->push(Zeta::Value::Null);
}

// 把 Zeta 值转换为反射 Any 值
Any toAny(Zeta::VM* vm, TypeID typeID, const Zeta::Value& value) {
    if(typeID == getTypeID<int>()) {
        auto val = value.as<int>();
        return val.has_value() ? Any(*val) : Any();
    }
    if(typeID == getTypeID<float>()) {
        auto val = value.as<float>();
        return val.has_value() ? Any(*val) : Any();
    }
    if(typeID == getTypeID<double>()) {
        auto val = value.as<double>();
        return val.has_value() ? Any(*val) : Any();
    }
    if(typeID == getTypeID<bool>()) {
        auto val = value.as<bool>();
        return val.has_value() ? Any(*val) : Any();
    }
    if(typeID == getTypeID<std::string>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(*val) : Any();
    }
    if(typeID == getTypeID<glm::vec2>()) {
        auto val = toVec2(vm, value);
        return val.has_value() ? Any(*val) : Any();
    }
    if(typeID == getTypeID<glm::vec3>()) {
        auto val = toVec3(vm, value);
        return val.has_value() ? Any(*val) : Any();
    }
    if(typeID == getTypeID<glm::vec4>()) {
        auto val = toVec4(vm, value);
        return val.has_value() ? Any(*val) : Any();
    }
    if(typeID == getTypeID<Color>()) {
        auto val = toColor(vm, value);
        return val.has_value() ? Any(*val) : Any();
    }
    if(typeID == getTypeID<ResPtr<Sprite>>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(ResPtr<Sprite>(*val)) : Any();
    }
    if(typeID == getTypeID<ResPtr<Texture2D>>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(ResPtr<Texture2D>(*val)) : Any();
    }
    if(typeID == getTypeID<ResPtr<AnimationClip>>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(ResPtr<AnimationClip>(*val)) : Any();
    }
    if(typeID == getTypeID<ResPtr<Font>>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(ResPtr<Font>(*val)) : Any();
    }
    if(typeID == getTypeID<ResPtr<Script>>()) {
        auto val = value.as<std::string>();
        return val.has_value() ? Any(ResPtr<Script>(*val)) : Any();
    }
    return Any();
}

// ---- Entity ----

void entityGetName(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    if(argc != 1 || !entity) {
        vm->reportError("Entity.get_name: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    vm->push(Zeta::Value(vm->internString(entity->getName())));
}

void entityGetTransform(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    if(argc != 1 || !entity) {
        vm->reportError("Entity.get_transform: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    wrapPtr(vm, &entity->getTransform(), ScriptBindings::transformClass);
}

void entityGetParent(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    if(argc != 1 || !entity) {
        vm->reportError("Entity.get_parent: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    wrapPtr(vm, entity->getParent(), ScriptBindings::entityClass);
}

void entityGetScene(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    if(argc != 1 || !entity) {
        vm->reportError("Entity.get_scene: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    wrapPtr(vm, entity->getScene(), ScriptBindings::sceneClass);
}

void entityIsAlive(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    if(argc != 1 || !entity) {
        vm->reportError("Entity.is_alive: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    vm->push(Zeta::Value(entity->isAlive()));
}

void entityDestroy(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    if(argc != 1 || !entity) {
        vm->reportError("Entity.destroy: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    entity->destroy();
    vm->push(Zeta::Value::Null);
}

void entityAddChild(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    auto name = vm->pop().as<std::string>();
    if(argc != 2 || !entity || !name.has_value()) {
        vm->reportError("Entity.add_child: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    wrapPtr(vm, entity->addChild(*name), ScriptBindings::entityClass);
}

void entityAddComponent(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    auto typeName = vm->pop().as<std::string>();
    if(argc != 2 || !entity || !typeName.has_value()) {
        vm->reportError("Entity.add_component: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    Component* comp = entity->addComponent(*typeName);
    wrapPtr(vm, comp, ScriptBindings::componentClass);
}

void entityRemoveComponent(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    auto typeName = vm->pop().as<std::string>();
    if(argc != 2 || !entity || !typeName.has_value()) {
        vm->reportError("Entity.remove_component: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    entity->removeComponent(*typeName);
    vm->push(Zeta::Value::Null);
}

void entityGetComponent(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    auto typeName = vm->pop().as<std::string>();
    if(argc != 2 || !entity || !typeName.has_value()) {
        vm->reportError("Entity.get_component: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    Component* comp = entity->getComponent(*typeName);
    wrapPtr(vm, comp, ScriptBindings::componentClass);
}

void entityHasComponent(Zeta::VM* vm, int argc) {
    Entity* entity = static_cast<Entity*>(vm->unwrapPointer());
    auto typeName = vm->pop().as<std::string>();
    if(argc != 2 || !entity || !typeName.has_value()) {
        vm->reportError("Entity.has_component: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    vm->push(Zeta::Value(entity->hasComponent(*typeName)));
}

// ---- Transform ----

void transformGetPos(Zeta::VM* vm, int argc) {
    Transform* transform = static_cast<Transform*>(vm->unwrapPointer());
    if(argc != 1 || !transform) {
        vm->reportError("Transform.get_pos: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    pushVec2(vm, transform->pos);
}

void transformSetPos(Zeta::VM* vm, int argc) {
    Transform* transform = static_cast<Transform*>(vm->unwrapPointer());
    auto posOpt = toVec2(vm, vm->pop());
    if(argc != 2 || !transform || !posOpt.has_value()) {
        vm->reportError("Transform.set_pos: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    transform->pos = *posOpt;
    vm->push(Zeta::Value::Null);
}

void transformGetRotation(Zeta::VM* vm, int argc) {
    Transform* transform = static_cast<Transform*>(vm->unwrapPointer());
    if(argc != 1 || !transform) {
        vm->reportError("Transform.get_rotation: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    vm->push(Zeta::Value(static_cast<double>(transform->rotation)));
}

void transformSetRotation(Zeta::VM* vm, int argc) {
    Transform* transform = static_cast<Transform*>(vm->unwrapPointer());
    auto rotationValue = vm->pop().as<float>();
    if(argc != 2 || !transform || !rotationValue.has_value()) {
        vm->reportError("Transform.set_rotation: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    transform->rotation = *rotationValue;
    vm->push(Zeta::Value::Null);
}

void transformGetScale(Zeta::VM* vm, int argc) {
    Transform* transform = static_cast<Transform*>(vm->unwrapPointer());
    if(argc != 1 || !transform) {
        vm->reportError("Transform.get_scale: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    pushVec2(vm, transform->scale);
}

void transformSetScale(Zeta::VM* vm, int argc) {
    Transform* transform = static_cast<Transform*>(vm->unwrapPointer());
    auto scaleOpt = toVec2(vm, vm->pop());
    if(argc != 2 || !transform || !scaleOpt.has_value()) {
        vm->reportError("Transform.set_scale: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    transform->scale = *scaleOpt;
    vm->push(Zeta::Value::Null);
}

// ---- Component 属性访问 ----

void componentGetTypeName(Zeta::VM* vm, int argc) {
    Component* component = static_cast<Component*>(vm->unwrapPointer());
    if(argc != 1 || !component) {
        vm->reportError("Component.get_type_name: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    Class* classInfo = ClassRegistry::get().getClass(component->getType());
    if(classInfo) {
        vm->newStrObj(classInfo->getName());
    } else {
        vm->push(Zeta::Value::Null);
    }
}

void componentGetProperty(Zeta::VM* vm, int argc) {
    Component* component = static_cast<Component*>(vm->unwrapPointer());
    auto name = vm->pop().as<std::string>();
    if(argc != 2 || !component || !name.has_value()) {
        vm->reportError("Component.get_property: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    Class* classInfo = ClassRegistry::get().getClass(component->getType());
    if(!classInfo) {
        vm->push(Zeta::Value::Null);
        return;
    }
    Property* property = classInfo->getProperty(*name);
    if(!property) {
        vm->push(Zeta::Value::Null);
        return;
    }
    Any value = property->getValue(component);
    assert(value.getID() == property->getTypeID());
    pushAny(vm, value);
}

void componentSetProperty(Zeta::VM* vm, int argc) {
    Component* component = static_cast<Component*>(vm->unwrapPointer());
    Zeta::Value val = vm->pop();
    auto name = vm->pop().as<std::string>();
    Class* classInfo = ClassRegistry::get().getClass(component->getType());
    if(!classInfo) {
        vm->push(Zeta::Value::Null);
        return;
    }
    Property* property = classInfo->getProperty(*name);
    if(!property) {
        vm->push(Zeta::Value::Null);
        return;
    }
    Any value = toAny(vm, property->getTypeID(), val);
    if (argc != 3 || !component || !name.has_value() || value.isNull()) {
        vm->reportError("Component.set_property: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    property->setValue(component, value);
    vm->push(Zeta::Value::Null);
}

// name, arg0, arg1, ..., argN, component
void componentCallMethod(Zeta::VM* vm, int argc) {
    Component* component = static_cast<Component*>(vm->unwrapPointer());
    auto methodName = vm->peek(-(argc - 1))->as<std::string>();
    if(argc < 2 || !component || !methodName.has_value()) {
        vm->pop(argc - 1);
        vm->reportError("Component.call_method: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    Class* classInfo = ClassRegistry::get().getClass(component->getType());
    if(!classInfo) {
        vm->push(Zeta::Value::Null);
        return;
    }
    Method* method = classInfo->getMethod(*methodName);
    if(!method) {
        vm->push(Zeta::Value::Null);
        return;
    }
    std::vector<Any> args(argc - 2);
    const std::vector<TypeID>& params = method->getParameters();
    if (params.size() != static_cast<size_t>(argc - 2)) {
        vm->pop(argc - 1);
        vm->reportError("Component.call_method: argument count mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    for(int i = 0; i < argc - 2; ++i) {
        Any arg = toAny(vm, params[i], *vm->peek(-static_cast<int>(argc - 2 - i)));
        if(arg.isNull()) {
            vm->pop(argc - 1);
            vm->reportError("Component.call_method: argument type mismatch");
            vm->push(Zeta::Value::Error);
            return;
        }
        args[i] = std::move(arg);
    }
    vm->pop(argc - 1);
    Any result = method->invoke(component, args);
    pushAny(vm, result);
}

// ---- Scene ----

void sceneFindEntity(Zeta::VM* vm, int argc) {
    Scene* scene = static_cast<Scene*>(vm->unwrapPointer());
    auto name = vm->pop().as<std::string>();
    if(argc != 2 || !scene || !name.has_value()) {
        vm->reportError("Scene.find_entity: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    Entity* entity = scene->getEntity(*name);
    wrapPtr(vm, entity, ScriptBindings::entityClass);
}

void sceneCreateEntity(Zeta::VM* vm, int argc) {
    Scene* scene = static_cast<Scene*>(vm->unwrapPointer());
    auto name = vm->pop().as<std::string>();
    if(argc != 2 || !scene || !name.has_value()) {
        vm->reportError("Scene.create_entity: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    Entity* entity = scene->createEntity(*name);
    wrapPtr(vm, entity, ScriptBindings::entityClass);
}

// ---- 全局函数: 输入 ----

void inputIsKeyPressed(Zeta::VM* vm, int argc) {
    auto key = vm->pop().as<int>();
    if (argc != 1 || !key.has_value()) {
        vm->reportError("cb_is_key_pressed: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    vm->push(Zeta::Value(Input::isKeyPressed(static_cast<KeyCode>(*key))));
}

void inputIsMouseButtonPressed(Zeta::VM* vm, int argc) {
    auto button = vm->pop().as<int>();
    if (argc != 1 || !button.has_value()) {
        vm->reportError("cb_is_mouse_button_pressed: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    vm->push(Zeta::Value(Input::isMouseButtonPressed(static_cast<MouseCode>(*button))));
}

void inputGetMousePos(Zeta::VM* vm, int argc) {
    if (argc != 0) {
        vm->reportError("cb_get_mouse_pos: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    pushVec2(vm, Input::getMousePos());
}

}  // namespace

void ScriptBindings::initialize(Zeta::VM& vm) {
    // 注册原生类; fields 为空, 引擎对象通过 _cpp_ptr 字段持有 C++ 指针
    entityClass = vm.registerClass("Entity", {
        {"_cpp_ptr", Zeta::Value::Null}
    }, {
        {"get_name", entityGetName},
        {"get_transform", entityGetTransform},
        {"get_parent", entityGetParent},
        {"get_scene", entityGetScene},
        {"is_alive", entityIsAlive},
        {"destroy", entityDestroy},
        {"add_child", entityAddChild},
        {"add_component", entityAddComponent},
        {"remove_component", entityRemoveComponent},
        {"get_component", entityGetComponent},
        {"has_component", entityHasComponent},
        {"_equal", equalFunction<Entity>},
    });
    transformClass = vm.registerClass("Transform", {
        {"_cpp_ptr", Zeta::Value::Null}
    }, {
        {"get_pos", transformGetPos},
        {"set_pos", transformSetPos},
        {"get_rotation", transformGetRotation},
        {"set_rotation", transformSetRotation},
        {"get_scale", transformGetScale},
        {"set_scale", transformSetScale},
        {"_equal", equalFunction<Transform>},
    });
    componentClass = vm.registerClass("Component", {
        {"_cpp_ptr", Zeta::Value::Null}
    }, {
        {"get_type_name", componentGetTypeName},
        {"get", componentGetProperty},
        {"set", componentSetProperty},
        {"call", componentCallMethod},
        {"_equal", equalFunction<Component>},
    }); 
    sceneClass = vm.registerClass("Scene", {
        {"_cpp_ptr", Zeta::Value::Null}
    }, {
        {"find_entity", sceneFindEntity},
        {"create_entity", sceneCreateEntity},
        {"_equal", equalFunction<Scene>},
    });

    vm.registerFunction("cb_is_key_pressed", inputIsKeyPressed);
    vm.registerFunction("cb_is_mouse_button_pressed", inputIsMouseButtonPressed);
    vm.registerFunction("cb_get_mouse_pos", inputGetMousePos);

    // TODO: 也可以改成直接内嵌字符串源码或者字节码
    auto cubeCoreScriptPath = vm.searchModuleFile("cube").first.string(); 
    std::unique_ptr<Zeta::Module> coreModule = ScriptRuntime::parseModule(cubeCoreScriptPath);
    vm.loadModule(coreModule.get());
    vec2Class = vm.findGlobal(coreModule->name, "Vec2");
    vec3Class = vm.findGlobal(coreModule->name, "Vec3");
    vec4Class = vm.findGlobal(coreModule->name, "Vec4");
}

void ScriptBindings::wrapEntity(Zeta::VM& vm, Entity* entity) {
    wrapPtr(&vm, entity, entityClass);
}

void ScriptBindings::wrapScene(Zeta::VM& vm, Scene* scene) {
    wrapPtr(&vm, scene, sceneClass);
}

}  // namespace Cube

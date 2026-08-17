#pragma once

#include "zeta/vm/vm.h"

namespace Cube {

class Entity;
class Scene;

// 向 Zeta VM 注册引擎原生类与全局函数, 使脚本可以操控实体与游戏逻辑
class ScriptBindings {
public:
    static void initialize(Zeta::VM& vm);

    // 供 ScriptComponent 在实例化脚本时把实体包装为 Zeta 对象, 包装结果压入栈顶
    static void wrapEntity(Zeta::VM& vm, Entity* entity);
    static void wrapScene(Zeta::VM& vm, Scene* scene);

    template<typename T>
    static void equalFunction(Zeta::VM* vm, int argc) {
        T* a = static_cast<T*>(vm->unwrapPointer());
        T* b = static_cast<T*>(vm->unwrapPointer());
        if(argc != 2 || !a || !b) {
            vm->reportError("equal: argument mismatch");
            vm->push(Zeta::Value::Error);
            return;
        }
        vm->push(Zeta::Value(a == b));
    }

    // 原生类全局索引缓存, 供 wrapPointer 使用
    static int entityClass;
    static int transformClass;
    static int componentClass;
    static int sceneClass;
    static int vec2Class;
    static int vec3Class;
    static int vec4Class;
};

}  // namespace Cube

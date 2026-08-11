#include "ScriptComponent.h"

#include "Cube/Core/Engine.h"
#include "Cube/Core/Log.h"

namespace Cube {

ScriptComponent::~ScriptComponent() {
    CB_CORE_INFO("ScriptComponent::~ScriptComponent()");
    // 组件销毁时释放脚本实例, 防止 Zeta VM 中的临时根泄漏
    if(!instance) {
        return;
    }
    if(Application* app = Engine::getApp()) {
        app->getScriptRuntime().getVM().discardInstance(instance);
    }
    instance = nullptr;
}

void ScriptComponent::start() {
    if(!script || !script->getModule()) {
        CB_CORE_ERROR("ScriptComponent::start(): script resource or module is null");
        return;
    }
    if(name.empty()) {
        CB_CORE_ERROR("ScriptComponent::start(): script class name is empty");
        return;
    }

    Zeta::VM& vm = Engine::getApp()->getScriptRuntime().getVM();
    const Zeta::Module* module = script->getModule();
    // 同一模块只会被加载一次, 重复调用是安全的
    vm.loadModule(module);

    classObj = vm.getGlobal(module->name, name);
    if(classObj.type != Zeta::Value::Type::Object ||
       classObj.ptrValue->type != Zeta::Object::Type::Class) {
        CB_CORE_ERROR("ScriptComponent::start(): class '{}' not found in module '{}'", name, module->name);
        return;
    }

    instance = vm.instantiate(classObj, 0, nullptr);
    if(!instance) {
        CB_CORE_ERROR("ScriptComponent::start(): failed to instantiate class '{}'", name);
        return;
    }
    // TODO: 脚本的 start 方法不存在时只会输出一次错误, 后续可改为先检查方法是否存在
    // NOTE: Zeta 的 callMethod 约定 args[0] 为实例自身, 且 argc 包含该参数
    Zeta::Value args[] = {*instance};
    vm.callMethod(*instance, "start", 1, args);
}

void ScriptComponent::update(float deltaTime) {
    if(!instance) {
        return;
    }
    Zeta::Value delta = Zeta::Value(static_cast<double>(deltaTime));
    Zeta::Value args[] = {*instance, delta};
    Engine::getApp()->getScriptRuntime().getVM().callMethod(*instance, "update", 2, args);
}

}  // namespace Cube

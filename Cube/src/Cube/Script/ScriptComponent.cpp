#include "ScriptComponent.h"

#include "Cube/Core/Engine.h"
#include "Cube/Core/Log.h"
#include "Cube/Script/ScriptBindings.h"

namespace Cube {

ScriptComponent::~ScriptComponent() {
    CB_CORE_INFO("ScriptComponent::~ScriptComponent()");
    // 组件销毁时释放脚本实例, 防止 Zeta VM 中的临时根泄漏
    if(!instance) {
        return;
    }
    if(Application* app = Engine::getApp()) {
        app->getScriptRuntime().getVM().popTempRoot(instance);
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

    int classIndex = vm.findGlobal(module->name, name);
    if(classIndex < 0) {
        CB_CORE_ERROR("ScriptComponent::start(): class '{}' not found in module '{}'", name, module->name);
        return;
    }
    // 实例化脚本类: 类对象与 _init(entity) 实参(实体包装)依次入栈
    ScriptBindings::wrapEntity(vm, getEntity());
    vm.push(vm.getGlobal(classIndex));
    vm.newInstance(1);
    // 实例通过临时根保存, 防止 GC 回收
    instance = vm.pushTempRoot();

    // 调用脚本 start(): 实例入栈后调用
    vm.push(*instance);
    vm.callMethod("start", 0);
    vm.pop();
}

void ScriptComponent::update(float deltaTime) {
    Zeta::VM& vm = Engine::getApp()->getScriptRuntime().getVM();
    vm.push(Zeta::Value(static_cast<double>(deltaTime)));
    vm.push(*instance);
    vm.callMethod("update", 1);
    vm.pop();
}

}  // namespace Cube

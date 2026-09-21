#include "ScriptComponent.h"

#include "Cube/Core/Engine.h"
#include "Cube/Core/Log.h"
#include "Cube/Script/ScriptBindings.h"

namespace Cube {

ScriptComponent::~ScriptComponent() {
    if (instance) {
        if (Application* app = Engine::getApp()) {
            app->getScriptRuntime().getVM().popTempRoot(instance);
        }
        instance = nullptr;
    }
}

void ScriptComponent::start() {
    if (!script || !script->getModule()) {
        CB_CORE_ERROR("ScriptComponent::start(): script resource or module is null");
        return;
    }
    if (name.empty()) {
        CB_CORE_ERROR("ScriptComponent::start(): script class name is empty");
        return;
    }

    Zeta::VM& vm = Engine::getApp()->getScriptRuntime().getVM();
    const Zeta::Module* module = script->getModule();
    vm.loadModule(module);

    int classIndex = vm.findGlobal(module->name, name);
    if (classIndex < 0) {
        CB_CORE_ERROR("ScriptComponent::start(): class '{}' not found in module '{}'", name, module->name);
        return;
    }

    ScriptBindings::wrapNode(vm, getNode());
    vm.push(vm.getGlobal(static_cast<uint32_t>(classIndex)));
    vm.newInstance(1);
    instance = vm.pushTempRoot();

    vm.push(*instance);
    vm.callMethod("start", 0);
    vm.pop();
}

void ScriptComponent::update(float deltaTime) {
    if (!instance) {
        return;
    }
    Zeta::VM& vm = Engine::getApp()->getScriptRuntime().getVM();
    vm.push(Zeta::Value(static_cast<double>(deltaTime)));
    vm.push(*instance);
    vm.callMethod("update", 1);
    vm.pop();
}

}  // namespace Cube

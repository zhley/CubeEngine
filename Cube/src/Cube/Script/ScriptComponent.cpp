#include "ScriptComponent.h"

#include "Cube/Core/Engine.h"
#include "Cube/Core/Log.h"
#include "Cube/Script/ScriptBindings.h"

namespace Cube {

ScriptComponent::ScriptComponent(Node* node, const std::string& scriptID, const std::string& name) : Component(node), script(scriptID), name(name) {
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

    ScriptBindings::wrapNode(vm, node);
    vm.push(vm.getGlobal(static_cast<uint32_t>(classIndex)));
    vm.newInstance(1);
    instance = vm.pushTempRoot();
}

ScriptComponent::~ScriptComponent() {
    if (instance) {
        if (Application* app = Engine::getApp()) {
            app->getScriptRuntime().getVM().popTempRoot(instance);
        }
        instance = nullptr;
    }
}

void ScriptComponent::start() {
    CB_ASSERT(instance);
    Zeta::VM& vm = Engine::getApp()->getScriptRuntime().getVM();
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

nlohmann::json ScriptComponent::serializeInstance() const {
    nlohmann::json data;
    Zeta::Instance* inst = *instance->as<Zeta::Instance*>();
    if (!inst) return data;
    inst->getFields()->forEach([&data](const Zeta::String* key, const Zeta::Value& value) {
        std::string keyName(key->getData(), key->getLength());
        if (keyName.starts_with("s_")) {
            keyName = keyName.substr(2);
            data[keyName] = ScriptBindings::serializeBasicValue(value);
        }
    });
    return data;
}

void ScriptComponent::deserializeInstance(const nlohmann::json &data) {
    Zeta::VM& vm = Engine::getApp()->getScriptRuntime().getVM();
    if (instance) {
        vm.popTempRoot(instance);
        instance = nullptr;
    }
    const Zeta::Module* module = script->getModule();
    int classIndex = vm.findGlobal(module->name, name);
    if (classIndex < 0) {
        CB_CORE_ERROR("ScriptComponent::deserializeInstance(): class '{}' not found in module '{}'", name, module->name);
        return;
    }
    ScriptBindings::wrapNode(vm, node);
    vm.push(vm.getGlobal(static_cast<uint32_t>(classIndex)));
    vm.newInstance(1);
    instance = vm.pushTempRoot();

    for (auto& [key, value] : data.items()) {
        std::string fieldName = "s_" + key;
        Zeta::Value zetaValue = ScriptBindings::deserializeBasicValue(value);
        (*instance->as<Zeta::Instance*>())->setField(vm.internString(fieldName), zetaValue);
    }
}

}  // namespace Cube

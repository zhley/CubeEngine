#include "ScriptRuntime.h"

#include <memory>
#include <string>
#include <string_view>

#include "zeta/compiler/compiler.h"
#include "zeta/compiler/bytecode.h"

#include "Cube/Core/Log.h"
#include "Cube/Script/ScriptBindings.h"

namespace Cube {

namespace {

void logNative(Zeta::VM* vm, int argc) {
    auto message = vm->pop().as<std::string_view>();
    if(argc != 1 || !message.has_value()) {
        vm->reportError("cb_log: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    CB_INFO("Zeta Log: {}", *message);
    vm->push(Zeta::Value::Null);
}

void logWarnNative(Zeta::VM* vm, int argc) {
    auto message = vm->pop().as<std::string_view>();
    if(argc != 1 || !message.has_value()) {
        vm->reportError("cb_log: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    CB_WARN("Zeta Log: {}", *message);
    vm->push(Zeta::Value::Null);
}

void logErrorNative(Zeta::VM* vm, int argc) {
    auto message = vm->pop().as<std::string_view>();
    if(argc != 1 || !message.has_value()) {
        vm->reportError("cb_log: argument mismatch");
        vm->push(Zeta::Value::Error);
        return;
    }
    CB_ERROR("Zeta Log: {}", *message);
    vm->push(Zeta::Value::Null);
}

}  // namespace

ScriptRuntime::ScriptRuntime(const std::vector<std::string>& moduleSearchPaths) : vm({1024, 8192, -1, moduleSearchPaths}) {
    vm.setErrorHandler([](const Zeta::VM::Error& error) {
        if (error.type == Zeta::VM::Error::RuntimeError) {
            CB_CORE_ERROR("[Runtime Error][line {} in {}]: {}", error.line, error.moduleName, error.message);
        } else {
            CB_CORE_ERROR("[VM Error]: {}", error.message);
        }
    });
    vm.registerFunction("cb_log", logNative);
    vm.registerFunction("cb_log_warn", logWarnNative);
    vm.registerFunction("cb_log_error", logErrorNative);
    ScriptBindings::initialize(vm);
}

ScriptRuntime::~ScriptRuntime() {
    ScriptBindings::shutdown();
}

std::unique_ptr<Zeta::Module> ScriptRuntime::parseModule(const Cube::Path& filePath) {
    if (filePath.extension() == ZETA_SRC_EXT) {
        std::string error;
        std::unique_ptr<Zeta::Module> module = Zeta::compileModule(filePath.string(), &error);
        if (!module) {
            CB_CORE_ERROR("ScriptRuntime::parseModule(): failed to compile module '{}': {}", filePath, error);
            return nullptr;
        }
        return module;
    } else if (filePath.extension() == ZETA_BC_EXT) {
        std::string error;
        std::unique_ptr<Zeta::Module> module = Zeta::deserializeModule(filePath.string(), &error);
        if (!module) {
            CB_CORE_ERROR("ScriptRuntime::parseModule(): failed to load bytecode module '{}': {}", filePath, error);
            return nullptr;
        }
        return module;
    } else {
        CB_CORE_ERROR("ScriptRuntime::parseModule(): unsupported file extension for '{}'", filePath);
        return nullptr;
    }
}

}  // namespace Cube

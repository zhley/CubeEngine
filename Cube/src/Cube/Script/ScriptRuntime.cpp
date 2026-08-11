#include "ScriptRuntime.h"

#include <memory>
#include <string>

#include "zeta/compiler/compiler.h"
#include "zeta/compiler/bytecode.h"

#include "Cube/Core/Log.h"

namespace Cube {

namespace {

Zeta::Value logNative(Zeta::VM* vm, int argc, Zeta::Value* argv) {
    if (argc != 1) {
        vm->reportError(std::format("Expected 1 argument, got {}", argc));
        return Zeta::Value::Error;
    }
    if(!argv[0].isString()) {
        vm->reportError("Expected a string argument");
        return Zeta::Value::Error;
    }
    Zeta::StrView message = argv[0].asString();
    CB_INFO("Zeta Log: {}", message.data);
    return Zeta::Value::Null;
}

Zeta::Value logWarnNative(Zeta::VM* vm, int argc, Zeta::Value* argv) {
    if (argc != 1) {
        vm->reportError(std::format("Expected 1 argument, got {}", argc));
        return Zeta::Value::Error;
    }
    if(!argv[0].isString()) {
        vm->reportError("Expected a string argument");
        return Zeta::Value::Error;
    }
    Zeta::StrView message = argv[0].asString();
    CB_WARN("Zeta Log: {}", message.data);
    return Zeta::Value::Null;
}

Zeta::Value logErrorNative(Zeta::VM* vm, int argc, Zeta::Value* argv) {
    if (argc != 1) {
        vm->reportError(std::format("Expected 1 argument, got {}", argc));
        return Zeta::Value::Error;
    }
    if(!argv[0].isString()) {
        vm->reportError("Expected a string argument");
        return Zeta::Value::Error;
    }
    Zeta::StrView message = argv[0].asString();
    CB_ERROR("Zeta Log: {}", message.data);
    return Zeta::Value::Null;
}

}  // namespace

ScriptRuntime::ScriptRuntime() : vm({1024, 8192, -1, {}}) {
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
}

std::unique_ptr<Zeta::Module> ScriptRuntime::parseModule(const std::string& filePath) {
    if (filePath.ends_with(ZETA_SRC_EXT)) {
        std::string error;
        std::unique_ptr<Zeta::Module> module = Zeta::compileModule(filePath, &error);
        if (!module) {
            CB_CORE_ERROR("ScriptRuntime::parseModule(): failed to compile module '{}': {}", filePath, error);
            return nullptr;
        }
        return module;
    } else if (filePath.ends_with(ZETA_BC_EXT)) {
        std::string error;
        std::unique_ptr<Zeta::Module> module = Zeta::deserializeModule(filePath, &error);
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

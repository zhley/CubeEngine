#pragma once

#include <memory>
#include <string>

#include "zeta/vm/vm.h"

namespace Cube {

// Owns the Zeta VM. Each Application has its own engine.
class ScriptRuntime {
public:
    ScriptRuntime();
    ~ScriptRuntime() = default;

    Zeta::VM& getVM() { return vm; }
    const Zeta::VM& getVM() const { return vm; }

    static std::unique_ptr<Zeta::Module> parseModule(const std::string& filePath);

private:
    Zeta::VM vm;
};

}  // namespace Cube

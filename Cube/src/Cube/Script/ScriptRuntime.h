#pragma once

#include <memory>
#include <string>

#include "zeta/vm/vm.h"

#include "Cube/Core/Path.h"

namespace Cube {

// TODO: 考虑到易用性, 或许可以给 Zeta 加一层封装

// Owns the Zeta VM. Each Application has its own engine.
class ScriptRuntime {
public:
    ScriptRuntime(const std::vector<std::string>& moduleSearchPaths);
    ~ScriptRuntime() = default;

    Zeta::VM& getVM() { return vm; }
    const Zeta::VM& getVM() const { return vm; }

    static std::unique_ptr<Zeta::Module> parseModule(const Cube::Path& filePath);

private:
    Zeta::VM vm;
};

}  // namespace Cube

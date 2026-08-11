#pragma once

#include <memory>

#include "zeta/compiler/bytecode.h"

#include "Resource.h"

namespace Cube {

class Script : public ResourceBase {
public:
    Script() = default;
    explicit Script(const std::string& filePath);

    const Zeta::Module* getModule() const { return module.get(); }

private:
    std::unique_ptr<Zeta::Module> module;
};

}  // namespace Cube

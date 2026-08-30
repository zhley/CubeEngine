#pragma once

#include <memory>

#include "zeta/compiler/bytecode.h"

#include "Cube/Core/Path.h"

#include "Resource.h"

namespace Cube {

class Script : public ResourceBase {
public:
    Script() = default;
    explicit Script(const Cube::Path& filePath);

    const Zeta::Module* getModule() const { return module.get(); }

private:
    std::unique_ptr<Zeta::Module> module;
};

}  // namespace Cube

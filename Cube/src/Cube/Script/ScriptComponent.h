#pragma once

#include "zeta/vm/value.h"

#include "Cube/Scene/Component.h"
#include "Cube/Resource/ResPtr.h"
#include "Cube/Resource/Script.h"

namespace Cube {

class ScriptComponent : public Component {
public:
    ResPtr<Script> script;
    std::string name;

    ScriptComponent() = default;
    ~ScriptComponent() override;

    void start() override;
    void update(float deltaTime) override;
    TypeID getType() const override { return getTypeID<ScriptComponent>(); }

private:
    // One Zeta instance per ScriptComponent; kept alive via a VM temp root.
    Zeta::Value* instance = nullptr;
};

}  // namespace Cube

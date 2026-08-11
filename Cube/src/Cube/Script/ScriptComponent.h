#pragma once

#include "Cube/Scene/Component.h"
#include "Cube/Resource/ResPtr.h"
#include "Cube/Resource/Script.h"

namespace Cube {

class ScriptRuntime;

class ScriptComponent : public Component {
public:
    ResPtr<Script> script;
    std::string name;

    ScriptComponent() = default;
    ~ScriptComponent() override = default;

    void start() override;
    void update(float deltaTime) override;
    TypeID getType() const override { return getTypeID<ScriptComponent>(); }

private:
    Zeta::Value classObj;
    Zeta::Value* instance;
};

}  // namespace Cube

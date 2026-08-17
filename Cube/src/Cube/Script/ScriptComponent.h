#pragma once

#include "zeta/vm/vm.h"

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
    ~ScriptComponent() override;

    void start() override;
    void update(float deltaTime) override;
    TypeID getType() const override { return getTypeID<ScriptComponent>(); }

private:
    // TODO: 一个实体上同种类型组件只能挂载一个, 要支持多脚本或多实例, 可以在 ScriptComponent 内部维护一个脚本实例列表.
    Zeta::Value* instance = nullptr;
};

}  // namespace Cube

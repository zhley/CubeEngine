#pragma once

#include "zeta/vm/value.h"
#include "json.hpp"

#include "Cube/Scene/Component.h"
#include "Cube/Resource/ResPtr.h"
#include "Cube/Resource/Script.h"

namespace Cube {

class ScriptComponent : public Component {
public:
    ScriptComponent(Node* node, const std::string& scriptID, const std::string& name);
    ~ScriptComponent() override;

    void start() override;
    void update(float deltaTime) override;
    TypeID getType() const override { return getTypeID<ScriptComponent>(); }

    const ResPtr<Script>& getScript() const { return script; }
    const std::string& getName() const { return name; }
    const Zeta::Value* getInstance() const { return instance; }

    nlohmann::json serializeInstance() const;
    void deserializeInstance(const nlohmann::json& data);

private:
    const ResPtr<Script> script;
    const std::string name;
    // One Zeta instance per ScriptComponent; kept alive via a VM temp root.
    Zeta::Value* instance = nullptr;
};

}  // namespace Cube

#pragma once

#include <string>
#include <unordered_map>

#include "Cube/Event/Event.h"

#include "View.h"

struct AssetNode;

namespace Cube {
class Event;
}

class ResourcesPanel : public View {
public:
    class ResourceUsageEvent : public Cube::Event {
    public:
        EVENT_TYPE(ResourceUsageEvent)

        ResourceUsageEvent(std::string identifier, bool inUse) : identifier(std::move(identifier)), inUse(inUse) {}

        std::string identifier;
        bool inUse = true;
    };

    ResourcesPanel(EditorPage& editorPage);
    ~ResourcesPanel() override = default;

    void render(float deltaTime) override;

private:
    bool onResourceUsage(const Cube::Event& e);

    // Reference count of the editors that are currently using each resource id.
    std::unordered_map<std::string, int> resourceUsageCounts;
};

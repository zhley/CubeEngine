#pragma once

#include <memory>

#include "json.hpp"

#include "Cube/Core/Path.h"
#include "Resource.h"

namespace Cube {
    
class Node;

// A .node resource: a node tree serialized to a file, used as a blueprint.
// It can be loaded through the ResourceManager with a "node:..." identifier
// or constructed directly from a file path. instantiate() deep-copies the
// blueprint into a fresh, detached subtree.
class NodeTree : public ResourceBase {
public:
    explicit NodeTree(const Cube::Path& filePath);
    ~NodeTree() = default;

    // Creates a detached copy of the blueprint. Returns nullptr on failure.
    std::unique_ptr<Node> instantiate() const;

    // Serializes a node tree to a .node file.
    static bool save(const Cube::Path& filePath, const Node& rootNode);

private:
    nlohmann::json data;
};

}

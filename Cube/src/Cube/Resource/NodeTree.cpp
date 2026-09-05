#include "NodeTree.h"

#include <fstream>

#include "json.hpp"

#include "Cube/Core/Log.h"
#include "Cube/Scene/Node.h"

namespace Cube {

NodeTree::NodeTree(const Cube::Path& filePath) {
    std::ifstream file(filePath.string());
    if(!file.is_open()) {
        CB_CORE_ERROR("NodeTree::NodeTree(): Failed to open node file '{}'", filePath);
        return;
    }
    file >> data;
    file.close();
}

std::unique_ptr<Node> NodeTree::instantiate() const {
    if(data.is_null() || !data.is_object()) {
        CB_CORE_ERROR("NodeTree::instantiate(): node file is empty or failed to parse");
        return nullptr;
    }
    std::unique_ptr<Node> root = std::make_unique<Node>();
    root->deserialize(data);
    return root;
}

bool NodeTree::save(const Cube::Path& filePath, const Node& rootNode) {
    std::ofstream file(filePath.string());
    if(!file.is_open()) {
        CB_CORE_ERROR("NodeTree::save(): Failed to open node file '{}'", filePath);
        return false;
    }
    file << rootNode.serialize().dump(4);
    file.close();
    return true;
}

}  // namespace Cube

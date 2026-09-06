#pragma once

namespace Cube {
    
class Node;

class RenderServer {
public:
    RenderServer() = default;
    ~RenderServer() = default;

    void renderNodeTree(Node* root);
};

}

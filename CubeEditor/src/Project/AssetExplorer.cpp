#include "AssetExplorer.h"

#include <fstream>

#include "Cube/Core/Log.h"
#include "Cube/Resource/ResourceManager.h"
#include "Cube/Core/Engine.h"

#include "../App/EditorApp.h"

void AssetExplorer::normalInit() {
    rootNode = std::make_unique<AssetNode>();
    rootNode->name = "root";
    rootNode->isGroup = true;
}

void AssetExplorer::loadFromFile(const Cube::Path& path, const Cube::Path& assetMapFilePath) {
    // Asset structure
    std::ifstream file(path.fspath());
    if(!file.is_open()) {
        CB_EDITOR_ERROR("Failed to open file {}", path);
        return;
    }
    nlohmann::json data;
    file >> data;
    file.close();
    rootNode = std::make_unique<AssetNode>();
    rootNode->fromJson(data);

    // AssetPathMap
    std::ifstream file2(assetMapFilePath.fspath());
    if(!file2.is_open()) {
        CB_EDITOR_ERROR("Failed to open AssetPathMapFile {}", assetMapFilePath);
        return;
    }
    nlohmann::json mapData;
    file2 >> mapData;
    assetPathMap = mapData;
    file2.close();
    resetResourceManager();
}

void AssetExplorer::saveToFile(const Cube::Path& path, const Cube::Path& assetMapFilePath) const {
    // Asset structure
    std::ofstream file(path.fspath());
    if(!file.is_open()) {
        CB_EDITOR_ERROR("Failed to open file {}", path);
        return;
    }
    file << rootNode->toJson().dump(4);
    file.close();

    // AssetPathMap
    std::ofstream file2(assetMapFilePath.fspath());
    if(!file2.is_open()) {
        CB_EDITOR_ERROR("Failed to open AssetPathMapFile {}", assetMapFilePath);
        return;
    }
    nlohmann::json mapData = assetPathMap;
    file2 << mapData.dump(4);
    file2.close();
}

AssetNode* AssetExplorer::getCurrentNode() const {
    return nodeStack.empty() ? rootNode.get() : nodeStack.top();
}

void AssetExplorer::enterNode(AssetNode* node) {
    if(node->isGroup) {
        nodeStack.push(node);
        currentPath += node->name + "/";
    }
}

void AssetExplorer::back() {
    if(!nodeStack.empty()) {
        nodeStack.pop();
        size_t pos = currentPath.find_last_of('/', currentPath.length() - 2);
        currentPath = currentPath.substr(0, pos + 1);
    }
}

void AssetExplorer::addNode(AssetNode* node) {
    node->parent = getCurrentNode();
    getCurrentNode()->children.push_back(std::unique_ptr<AssetNode>(node));
}

AssetNode* AssetExplorer::createGroup(const std::string& name) {
    AssetNode* node = new AssetNode{name, "", Cube::ResourceType::Unknown, true, {}};
    addNode(node);
    return node;
}

void AssetExplorer::createResource(const std::string& identifier, const nlohmann::json& content) {
    addNode(new AssetNode{identifier, identifier, Cube::getResType(identifier), false, {}});
    assetPathMap[identifier] = content;
    resetResourceManager();
}

void AssetExplorer::reimportResource(const std::string& identifier, const nlohmann::json& content) {
    assetPathMap[identifier] = content;
    resetResourceManager();
}

void AssetExplorer::removeNode(AssetNode* node) {
    _removeNode(node);
    resetResourceManager();
}

void AssetExplorer::move(const AssetNode* src, AssetNode* dst) {
    if(!dst->isGroup) return;
    auto& vec = src->parent->children;
    auto it = std::find_if(vec.begin(), vec.end(), [src](const std::unique_ptr<AssetNode>& n){ return n.get() == src; });
    if(it == vec.end()) return;
    dst->children.push_back(std::move(*it));
    vec.erase(it);
}

void AssetExplorer::_removeNode(AssetNode* node) {
    if(node->isGroup) {
        enterNode(node);
        for(auto& c : node->children) {
            removeNode(c.get());
        }
        back();
    }else {
        assetPathMap.erase(node->identifier);
    }
    auto& vec = getCurrentNode()->children;
    auto it = std::find_if(vec.begin(), vec.end(), [node](const std::unique_ptr<AssetNode>& n) { return n.get() == node; });
    if(it != vec.end()){
        vec.erase(it);
    }
}

void AssetExplorer::resetResourceManager() {
    Cube::Engine::getApp()->getResourceManager().reset(assetPathMap);
}

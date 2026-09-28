#include "Project.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <json.hpp>

#include "Cube/Core/Log.h"
#include "Cube/Core/Path.h"
#include "Cube/Resource/NodeTree.h"
#include "Cube/Resource/ResourceManager.h"
#include "Cube/Scene/Node.h"
#include "Cube/Utils/Utils.h"

Project::Project(const std::string& name, const Cube::Path& rootPath) {
    config.name = name;
    config.rootPath = rootPath;
    config.projectDataDirectory = rootPath / ".cube";
    config.assetsDirectory = rootPath / "Assets";
    config.assetPathMapFilePath = config.rootPath / "asset.json";

    std::filesystem::create_directories(config.projectDataDirectory.fspath());
    std::filesystem::create_directories(config.assetsDirectory.fspath());
    writeToConfigFile(rootPath / (name + ".cbproj"));

    assetExplorer.normalInit();
}

Project::Project(const Cube::Path& configFilePath) {
    nlohmann::json data;
    std::ifstream file(configFilePath.fspath());
    if(!file.is_open()) {
        CB_ERROR("Project::Project: Failed to open file: {}", configFilePath);
        CB_ASSERT(false);
        return;
    }
    file >> data;
    file.close();

    config.name = data["name"];
    config.rootPath = configFilePath.parentPath();
    config.projectDataDirectory = config.rootPath / ".cube";
    config.assetsDirectory = config.rootPath / "Assets";
    config.assetPathMapFilePath = config.rootPath / "asset.json";
    load();
}

Project::~Project() {
    save();
}

const std::deque<NodeDocument>& Project::getDocuments() const { return documents; }

std::deque<NodeDocument>& Project::getDocuments() { return documents; }

void Project::addDocument(const std::string& identifier, std::unique_ptr<Cube::Node> root) {
    documents.push_back({identifier, std::move(root), false});
}

bool Project::hasDocument(const std::string& identifier) const {
    return std::any_of(documents.begin(), documents.end(), [&identifier](const NodeDocument& doc) { return doc.identifier == identifier; });
}

Cube::Path Project::resolveResourcePath(const std::string& identifier) const {
    try {
        return Cube::Path(assetExplorer.getAssetImporter(identifier).at("path").get<std::string>());
    } catch (const std::exception& e) {
        CB_ERROR("Project::resolveResourcePath: failed to resolve '{}': {}", identifier, e.what());
        return Cube::Path();
    }
}


void importTexture(const Cube::Path& texturePath, Project* project, AssetExplorer& assetExplorer) {
    Cube::Path relPath = texturePath.lexicallyRelative(project->getConfig().assetsDirectory);
    nlohmann::json importConfig;
    importConfig["path"] = texturePath.string();
    assetExplorer.createResource("tex:" + relPath.string(), importConfig);
}

void importAnimClip(const Cube::Path& animPath, Project* project, AssetExplorer& assetExplorer) {
    Cube::Path relPath = animPath.lexicallyRelative(project->getConfig().assetsDirectory);
    nlohmann::json importConfig;
    importConfig["path"] = animPath.string();
    assetExplorer.createResource("anim:" + relPath.string(), importConfig);
}

void importNodeTree(const Cube::Path& nodePath, Project* project, AssetExplorer& assetExplorer) {
    Cube::Path relPath = nodePath.lexicallyRelative(project->getConfig().assetsDirectory);
    nlohmann::json importConfig;
    importConfig["path"] = nodePath.string();
    assetExplorer.createResource("node:" + relPath.string(), importConfig);
}

void importRes(const Cube::Path& source, const Cube::Path& target, Project* project, AssetExplorer& assetExplorer) {
    std::filesystem::path sourcePath = source.fspath();
    std::filesystem::path targetPath = target.fspath();
    if(std::filesystem::is_directory(sourcePath)) {
        std::filesystem::create_directories(targetPath);
        for(const auto& entry : std::filesystem::directory_iterator(sourcePath)) {
            Cube::Path child(entry.path().string());
            importRes(child, target / child.filename(), project, assetExplorer);
        }
    }else {
        if(target != source) {
            std::error_code ec;
            // TODO: 允许覆盖, 但是给出确认弹窗
            std::filesystem::copy_file(source.fspath(), target.fspath(), std::filesystem::copy_options::none, ec);
            if(ec) {
                CB_EDITOR_ERROR("Failed to copy file from {} to {}. Error Code: {}", source, target, ec.message());
                return;
            }
        }
        const std::string_view extension = target.extension();
        if(extension == ".png" || extension == ".jpg") {
            importTexture(target, project, assetExplorer);
        } else if(extension == ".anim") {
            importAnimClip(target, project, assetExplorer);
        } else if(extension == ".node") {
            importNodeTree(target, project, assetExplorer);
        }
        else {
            CB_EDITOR_ERROR("Unknown assets format: {}", source.extension());
            return;
        }
    }
}

void Project::importResource(const Cube::Path& path) {
    importRes(path, config.assetsDirectory / path.filename(), this, assetExplorer);
}

const ProjectConfig& Project::getConfig() const {
    return config;
}

void Project::save() {

    nlohmann::json data;
    data["nodes"] = nlohmann::json::array();
    for(auto& doc : documents) {
        data["nodes"].push_back(doc.identifier);
    }

    const Cube::Path nodesCacheFile = config.projectDataDirectory / "nodes.cache";
    std::ofstream file(nodesCacheFile.fspath());
    if(!file.is_open()) {
        CB_ERROR("Project::save: failed to open file: {}", nodesCacheFile);
        CB_ASSERT(false);
        return;
    }
    file << data.dump(4);
    file.close();

    assetExplorer.saveToFile(config.projectDataDirectory / "resources.cache", config.assetPathMapFilePath);
}

void Project::writeToConfigFile(const Cube::Path& configFilePath) const {
    nlohmann::json data;
    data["name"] = config.name;

    std::ofstream file(configFilePath.fspath());
    if(!file.is_open()) {
        CB_ERROR("Project::Project: Failed to open file: {}", configFilePath);
        CB_ASSERT(false);
        return;
    }
    file << data.dump(4);
}

void Project::load() {
    // resources.cache
    assetExplorer.loadFromFile(config.projectDataDirectory / "resources.cache", config.assetPathMapFilePath);

    // nodes.cache
    const Cube::Path nodesCacheFile = config.projectDataDirectory / "nodes.cache";
    std::ifstream file(nodesCacheFile.fspath());
    if(!file.is_open()) {
        CB_ERROR("Project::load: failed to open file: {}", nodesCacheFile);
        CB_ASSERT(false);
        return;
    }
    nlohmann::json data;
    file >> data;
    for(auto& s : data["nodes"]) {
        const std::string identifier = s.get<std::string>();
        const Cube::Path path = resolveResourcePath(identifier);
        if(path.empty()) {
            CB_ERROR("Project::load: failed to resolve node document '{}'", identifier);
            continue;
        }
        Cube::NodeTree nodeTree(path);
        std::unique_ptr<Cube::Node> root = nodeTree.instantiate();
        if(!root) {
            CB_ERROR("Project::load: failed to load node document '{}'", identifier);
            continue;
        }
        documents.push_back({identifier, std::move(root), true});
    }
    file.close();
}

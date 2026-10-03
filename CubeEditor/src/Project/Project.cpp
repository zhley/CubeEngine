#include "Project.h"

#include <filesystem>
#include <fstream>
#include <json.hpp>

#include "Cube/Core/Log.h"
#include "Cube/Core/Path.h"
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
    assetExplorer.loadFromFile(config.projectDataDirectory / "resources.cache", config.assetPathMapFilePath);
}

Project::~Project() {
    save();
}

std::string importTexture(const Cube::Path& texturePath, Project* project, AssetExplorer& assetExplorer) {
    Cube::Path relPath = texturePath.lexicallyRelative(project->getConfig().assetsDirectory);
    nlohmann::json importConfig;
    importConfig["path"] = texturePath.string();
    std::string identifier = "tex:" + relPath.string();
    assetExplorer.createResource(identifier, importConfig);
    return identifier;
}

std::string importAnimClip(const Cube::Path& animPath, Project* project, AssetExplorer& assetExplorer) {
    Cube::Path relPath = animPath.lexicallyRelative(project->getConfig().assetsDirectory);
    nlohmann::json importConfig;
    importConfig["path"] = animPath.string();
    std::string identifier = "anim:" + relPath.string();
    assetExplorer.createResource(identifier, importConfig);
    return identifier;
}

std::string importNodeTree(const Cube::Path& nodePath, Project* project, AssetExplorer& assetExplorer) {
    Cube::Path relPath = nodePath.lexicallyRelative(project->getConfig().assetsDirectory);
    nlohmann::json importConfig;
    importConfig["path"] = nodePath.string();
    std::string identifier = "node:" + relPath.string();
    assetExplorer.createResource(identifier, importConfig);
    return identifier;
}

std::string importScript(const Cube::Path& scriptPath, Project* project, AssetExplorer& assetExplorer) {
    Cube::Path relPath = scriptPath.lexicallyRelative(project->getConfig().assetsDirectory);
    nlohmann::json importConfig;
    importConfig["path"] = scriptPath.string();
    std::string identifier = "script:" + relPath.string();
    assetExplorer.createResource(identifier, importConfig);
    return identifier;
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
        } else if(extension == ".zt" || extension == ".ztc") {
            importScript(target, project, assetExplorer);
        }
        else {
            CB_EDITOR_ERROR("Unknown assets format: {}", source.extension());
            return;
        }
    }
}

std::string Project::importResource(const Cube::Path& filePath) {
    if(!std::filesystem::is_regular_file(filePath.fspath())) {
        CB_EDITOR_ERROR("Project::importResource: not a file: {}", filePath);
        return {};
    }

    const Cube::Path target = config.assetsDirectory / filePath.filename();
    if(target != filePath) {
        std::error_code ec;
        // TODO: 允许覆盖, 但是给出确认弹窗
        std::filesystem::copy_file(filePath.fspath(), target.fspath(), std::filesystem::copy_options::none, ec);
        if(ec) {
            CB_EDITOR_ERROR("Failed to copy file from {} to {}. Error Code: {}", filePath, target, ec.message());
            return {};
        }
    }

    const std::string_view extension = target.extension();
    if(extension == ".png" || extension == ".jpg") {
        return importTexture(target, this, assetExplorer);
    }
    if(extension == ".anim") {
        return importAnimClip(target, this, assetExplorer);
    }
    if(extension == ".node") {
        return importNodeTree(target, this, assetExplorer);
    }
    if(extension == ".zt" || extension == ".ztc") {
        return importScript(target, this, assetExplorer);
    }
    CB_EDITOR_ERROR("Unknown assets format: {}", filePath);
    return {};
}

void Project::importResources(const Cube::Path& path) {
    importRes(path, config.assetsDirectory / path.filename(), this, assetExplorer);
}

const ProjectConfig& Project::getConfig() const {
    return config;
}

void Project::save() {
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

#include "Project.h"

#include <filesystem>
#include <fstream>
#include <json.hpp>

#include "Cube/Core/Log.h"
#include "Cube/Core/Path.h"
#include "Cube/Resource/ResourceManager.h"
#include "Cube/Utils/Utils.h"

Project::Project(const std::string& name, const Cube::Path& rootPath) {
    config.name = name;
    config.rootPath = rootPath;
    config.projectDataDirectory = rootPath / ".cube";
    config.assetsDirectory = rootPath / "Assets";
    config.sceneDirectory = rootPath / "Scenes";
    config.assetPathMapFilePath = config.rootPath / "AssetMap.json";

    std::filesystem::create_directories(config.projectDataDirectory.string());
    std::filesystem::create_directories(config.sceneDirectory.string());
    std::filesystem::create_directories(config.assetsDirectory.string());
    writeToConfigFile(rootPath / (name + ".cbproj"));

    assetExplorer.normalInit();
}

Project::Project(const Cube::Path& configFilePath) {
    nlohmann::json data;
    std::ifstream file(configFilePath.string());
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
    config.sceneDirectory = config.rootPath / "Scenes";
    config.assetPathMapFilePath = config.rootPath / "AssetMap.json";
    load();
}

Project::~Project() {
    save();

    for(auto s : scenes) {
        delete s.scene;
    }
}

const std::vector<SceneData>& Project::getScenes() const { return scenes; }

std::vector<SceneData>& Project::getScenes() { return scenes; }

void Project::addScene(Cube::Scene* scene) {
    scenes.push_back({scene, false});
}

bool Project::hasScene(const std::string& sceneName) const {
    auto it = std::find_if(scenes.begin(), scenes.end(), [sceneName](SceneData s) { return s.scene->getName() == sceneName; });
    return it != scenes.end();
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

void importRes(const Cube::Path& source, const Cube::Path& target, Project* project, AssetExplorer& assetExplorer) {
    std::filesystem::path sourcePath = source.string();
    std::filesystem::path targetPath = target.string();
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
            std::filesystem::copy_file(source.string(), target.string(), std::filesystem::copy_options::none, ec);
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
    data["scenes"] = nlohmann::json::array();
    for(auto& s : scenes) {
        data["scenes"].push_back(s.scene->getName());
    }

    const Cube::Path scenesCacheFile = config.projectDataDirectory / "scenes.cache";
    std::ofstream file(scenesCacheFile.string());
    if(!file.is_open()) {
        CB_ERROR("Project::save: failed to open file: {}", scenesCacheFile);
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

    std::ofstream file(configFilePath.string());
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

    // scenes.cache
    const Cube::Path scenesCacheFile = config.projectDataDirectory / "scenes.cache";
    std::ifstream file(scenesCacheFile.string());
    if(!file.is_open()) {
        CB_ERROR("Project::load: failed to open file: {}", scenesCacheFile);
        CB_ASSERT(false);
        return;
    }
    nlohmann::json data;
    file >> data;
    for(auto& s : data["scenes"]) {
        Cube::Scene* scene = new Cube::Scene((config.sceneDirectory / (s.get<std::string>() + ".scene")).string());
        scenes.push_back({scene, true});
    }
    file.close();
}

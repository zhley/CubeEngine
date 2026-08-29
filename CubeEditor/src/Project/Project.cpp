#include "Project.h"

#include <filesystem>
#include <fstream>
#include <json.hpp>

#include "Cube/Core/Log.h"
#include "Cube/Resource/ResourceManager.h"
#include "Cube/Utils/Utils.h"

Project::Project(const std::string& name, const std::string& rootPath) {
    config.name = name;
    config.rootPath = rootPath;
    config.projectDataDirectory = rootPath + "/.cube";
    config.assetsDirectory = rootPath + "/Assets";
    config.sceneDirectory = rootPath + "/Scenes";
    config.assetPathMapFilePath = config.rootPath + "/AssetMap.json";

    std::filesystem::create_directories(config.projectDataDirectory);
    std::filesystem::create_directories(config.sceneDirectory);
    std::filesystem::create_directories(config.assetsDirectory);
    writeToConfigFile(rootPath + "/" + name + ".cbproj");

    assetExplorer.normalInit();
}

Project::Project(const std::string& configFilePath) {
    nlohmann::json data;
    std::ifstream file(configFilePath);
    if(!file.is_open()) {
        CB_ERROR("Project::Project: Failed to open file: {}", configFilePath);
        CB_ASSERT(false);
        return;
    }
    file >> data;
    file.close();

    config.name = data["name"];
    config.rootPath = std::filesystem::path(configFilePath).parent_path().string();
    config.projectDataDirectory = config.rootPath + "/.cube";
    config.assetsDirectory = config.rootPath + "/Assets";
    config.sceneDirectory = config.rootPath + "/Scenes";
    config.assetPathMapFilePath = config.rootPath + "/AssetMap.json";
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


void importTexture(const std::filesystem::path& texturePath, Project* project, AssetExplorer& assetExplorer) {
    std::filesystem::path path = std::filesystem::canonical(texturePath);
    std::filesystem::path relPath = std::filesystem::relative(path, project->getConfig().assetsDirectory);
    nlohmann::json importConfig;
    importConfig["path"] = path.generic_string();
    assetExplorer.createResource("tex:" + relPath.generic_string(), importConfig);
}

void importAnimClip(const std::filesystem::path& animPath, Project* project, AssetExplorer& assetExplorer) {
    std::filesystem::path path = std::filesystem::canonical(animPath);
    std::filesystem::path relPath = std::filesystem::relative(path, project->getConfig().assetsDirectory);
    nlohmann::json importConfig;
    importConfig["path"] = path.generic_string();
    assetExplorer.createResource("anim:" + relPath.generic_string(), importConfig);
}

void importRes(const std::filesystem::path& source, const std::filesystem::path& target, Project* project, AssetExplorer& assetExplorer) {
    if(std::filesystem::is_directory(source)) {
        if(!std::filesystem::exists(target)) {
            std::filesystem::create_directories(target);
        }
        for(auto& entry : std::filesystem::directory_iterator(source)) {
            importRes(entry.path(), target / entry.path().filename(), project, assetExplorer);
        }
    }else {
        if(target != source) {
            std::error_code ec;
            std::filesystem::copy_file(source, target, std::filesystem::copy_options::none, ec);
            if(ec) {
                CB_EDITOR_ERROR("Failed to copy file from {} to {}. Error Code: {}", source.string(), target.string(), ec.message());
                return;
            }
        }
        if(target.extension() == ".png" || target.extension() == ".jpg") {
            importTexture(target, project, assetExplorer);
        } else if(target.extension() == ".anim") {
            importAnimClip(target, project, assetExplorer);
        }
        else {
            CB_EDITOR_ERROR("Unknown assets format: {}", source.extension().string());
            return;
        }
    }
}

void Project::importResource(const std::string& path) {
    std::filesystem::path filepath(path);
    std::filesystem::path targetFile = config.assetsDirectory;
    targetFile /= filepath.filename();
    importRes(path, targetFile, this, assetExplorer);
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

    std::ofstream file(config.projectDataDirectory + "/scenes.cache");
    if(!file.is_open()) {
        CB_ERROR("Project::save: failed to open file: {}", config.projectDataDirectory + "/scenes.cache");
        CB_ASSERT(false);
        return;
    }
    file << data.dump(4);
    file.close();

    assetExplorer.saveToFile(config.projectDataDirectory + "/resources.cache", config.assetPathMapFilePath);
}

void Project::writeToConfigFile(const std::string& configFilePath) const {
    nlohmann::json data;
    data["name"] = config.name;

    std::ofstream file(configFilePath);
    if(!file.is_open()) {
        CB_ERROR("Project::Project: Failed to open file: {}", configFilePath);
        CB_ASSERT(false);
        return;
    }
    file << data.dump(4);
}

void Project::load() {
    // resources.cache
    assetExplorer.loadFromFile(config.projectDataDirectory + "/resources.cache", config.assetPathMapFilePath);

    // scenes.cache
    std::ifstream file(config.projectDataDirectory + "/scenes.cache");
    if(!file.is_open()) {
        CB_ERROR("Project::load: failed to open file: {}", config.projectDataDirectory + "/scenes.cache");
        CB_ASSERT(false);
        return;
    }
    nlohmann::json data;
    file >> data;
    for(auto& s : data["scenes"]) {
        Cube::Scene* scene = new Cube::Scene(config.sceneDirectory + "/" + s.get<std::string>() + ".scene");
        scenes.push_back({scene, true});
    }
    file.close();
}

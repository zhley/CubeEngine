#pragma once

#include <string>

#include "AssetExplorer.h"
#include "Cube/Scene/Scene.h"

struct ProjectConfig {
	std::string name;
	std::string rootPath; // project root directory
	std::string projectDataDirectory;
	std::string assetsDirectory;
	std::string sceneDirectory;
	std::string assetPathMapFilePath;
};

struct SceneData {
    Cube::Scene* scene = nullptr;
	bool isSaved = false;
};

class Project final{
public:
    Project(const std::string& name, const std::string& rootPath);
	Project(const std::string& configFilePath);
	~Project();

	const std::vector<SceneData>& getScenes() const;
	std::vector<SceneData>& getScenes();
	void addScene(Cube::Scene* scene);
	bool hasScene(const std::string& sceneName) const;

	void importResource(const std::string& path);

	const ProjectConfig& getConfig() const;
    AssetExplorer& getAssetExplorer() { return assetExplorer; }

private:
	void writeToConfigFile(const std::string& configFilePath) const;
	void load();
	void save();

	ProjectConfig config;
    std::vector<SceneData> scenes;
	AssetExplorer assetExplorer;
};

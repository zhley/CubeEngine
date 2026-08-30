#pragma once

#include <string>

#include "AssetExplorer.h"
#include "Cube/Core/Path.h"
#include "Cube/Scene/Scene.h"

struct ProjectConfig {
	std::string name;
	Cube::Path rootPath; // project root directory
	Cube::Path projectDataDirectory;
	Cube::Path assetsDirectory;
	Cube::Path sceneDirectory;
	Cube::Path assetPathMapFilePath;
};

struct SceneData {
    Cube::Scene* scene = nullptr;
	bool isSaved = false;
};

class Project final{
public:
    Project(const std::string& name, const Cube::Path& rootPath);
	Project(const Cube::Path& configFilePath);
	~Project();

	const std::vector<SceneData>& getScenes() const;
	std::vector<SceneData>& getScenes();
	void addScene(Cube::Scene* scene);
	bool hasScene(const std::string& sceneName) const;

	void importResource(const Cube::Path& path);

	const ProjectConfig& getConfig() const;
    AssetExplorer& getAssetExplorer() { return assetExplorer; }

private:
	void writeToConfigFile(const Cube::Path& configFilePath) const;
	void load();
	void save();

	ProjectConfig config;
    std::vector<SceneData> scenes;
	AssetExplorer assetExplorer;
};

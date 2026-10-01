#pragma once

#include <memory>
#include <string>
#include <vector>

#include "AssetExplorer.h"
#include "Cube/Core/Path.h"

namespace Cube {
class Node;
}

struct ProjectConfig {
    std::string name;
    Cube::Path rootPath; // project root directory
    Cube::Path projectDataDirectory;
    Cube::Path assetsDirectory;
    Cube::Path assetPathMapFilePath;
};

class Project final{
public:
    Project(const std::string& name, const Cube::Path& rootPath);
    Project(const Cube::Path& configFilePath);
    ~Project();

    std::string importResource(const Cube::Path& filePath);
    void importResources(const Cube::Path& path);

    const ProjectConfig& getConfig() const;
    AssetExplorer& getAssetExplorer() { return assetExplorer; }

private:
    void writeToConfigFile(const Cube::Path& configFilePath) const;
    void save();

    ProjectConfig config;
    AssetExplorer assetExplorer;
};

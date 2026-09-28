#pragma once

#include <deque>
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

struct NodeDocument {
    std::string identifier;
    std::unique_ptr<Cube::Node> root;
	bool isSaved = false;
};

class Project final{
public:
    Project(const std::string& name, const Cube::Path& rootPath);
	Project(const Cube::Path& configFilePath);
	~Project();

	const std::deque<NodeDocument>& getDocuments() const;
	// deque: push_back does not move existing elements, so NodeDocument* stays valid.
	std::deque<NodeDocument>& getDocuments();
	void addDocument(const std::string& identifier, std::unique_ptr<Cube::Node> root);
	bool hasDocument(const std::string& identifier) const;

	// Resolve a resource identifier to its file path via the asset map.
	Cube::Path resolveResourcePath(const std::string& identifier) const;

	void importResource(const Cube::Path& path);

	const ProjectConfig& getConfig() const;
    AssetExplorer& getAssetExplorer() { return assetExplorer; }

private:
	void writeToConfigFile(const Cube::Path& configFilePath) const;
	void load();
	void save();

	ProjectConfig config;
    std::deque<NodeDocument> documents;
	AssetExplorer assetExplorer;
};

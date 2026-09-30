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

// In-memory editable NodeTree instance. Owned by the editor session, not the project.
class NodeDocument {
public:
	NodeDocument(std::string identifier, std::unique_ptr<Cube::Node> root);

	const std::string& getIdentifier() const { return identifier; }
	Cube::Node* getRoot() const { return root.get(); }
	bool isDirty() const { return dirty; }
	void markDirty() { dirty = true; }
	void markSaved() { dirty = false; }

private:
	std::string identifier;
	std::unique_ptr<Cube::Node> root;
	bool dirty = false;
};

class Project final{
public:
    Project(const std::string& name, const Cube::Path& rootPath);
	Project(const Cube::Path& configFilePath);
	~Project();

	void importResource(const Cube::Path& path);

	// NodeTree files under Assets
	bool createNodeTreeFile(const std::string& name); // "node:<name>.node"
	bool saveNodeTree(const std::string& identifier, const Cube::Node& root);
	std::unique_ptr<Cube::Node> loadNodeTree(const Cube::Path& filePath) const;

	const ProjectConfig& getConfig() const;
    AssetExplorer& getAssetExplorer() { return assetExplorer; }

private:
	void writeToConfigFile(const Cube::Path& configFilePath) const;
	void save();

	ProjectConfig config;
	AssetExplorer assetExplorer;
};

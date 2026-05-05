#include "misc.h"

#include "Cube/Core/Log.h"
#include "json.hpp"

#include <fstream>

namespace Utils{
	std::string getUserConfigDir() {
	#ifdef _WIN32
		std::string dir = std::string(std::getenv("APPDATA")); // C:\Users\UserName\AppData\Roaming
		if(dir.empty()) {
			CB_EDITOR_ERROR("getUserConfigDir: Failed to get environment variable \"APPDATA\"");
		}
		return dir;
	#elif __APPLE__
		std::string dir = std::string(std::getenv("HOME"));
		if(dir.empty()) {
			CB_EDITOR_ERROR("getUserConfigDir: Failed to get environment variable \"HOME\"");
		}
		return dir + "/Library/Application Support";
	#elif __Linux__
		std::string dir = std::string(std::getenv("XDG_CONFIG_HOME"));
		if(!dir.empty()) {
			return dir;
		}else {
			dir = std::getenv("HOME");
			if(dir.empty()) {
				CB_EDITOR_ERROR("getUserConfigDir: Failed to get environment variable \"HOME\"");
			}
			return dir + "/.config";
		}
	#else
		CB_EDITOR_ERROR("Unknown operating system")
		return "";
	#endif
	}

	nlohmann::json parseAtlasFile(const std::string &filePath){
		nlohmann::json sprites = nlohmann::json::object();

		std::ifstream file(filePath);
		if(!file.is_open()) {
			CB_EDITOR_ERROR("parseAtlasFile: Failed to open atlas file {}", filePath);
			return sprites;
		}

		nlohmann::json atlasJson;
		try {
			file >> atlasJson;
		} catch(const nlohmann::json::exception& e) {
			CB_EDITOR_ERROR("parseAtlasFile: Invalid atlas json in {}: {}", filePath, e.what());
			return sprites;
		}

		if(!atlasJson.contains("meta") || !atlasJson["meta"].contains("size") || !atlasJson["meta"]["size"].contains("w") || !atlasJson["meta"]["size"].contains("h")) {
			CB_EDITOR_ERROR("parseAtlasFile: Missing meta.size in {}", filePath);
			return sprites;
		}

		const float atlasW = atlasJson["meta"]["size"]["w"].get<float>();
		const float atlasH = atlasJson["meta"]["size"]["h"].get<float>();
		if(atlasW <= 0.0f || atlasH <= 0.0f) {
			CB_EDITOR_ERROR("parseAtlasFile: Invalid atlas size ({}, {}) in {}", atlasW, atlasH, filePath);
			return sprites;
		}

		if(!atlasJson.contains("frames") || !atlasJson["frames"].is_array()) {
			CB_EDITOR_ERROR("parseAtlasFile: Missing frames array in {}", filePath);
			return sprites;
		}

		for(const auto& frame : atlasJson["frames"]) {
			if(!frame.contains("filename") || !frame.contains("frame") || !frame["frame"].is_object()) {
				continue;
			}

			const auto& rect = frame["frame"];
			if(!rect.contains("x") || !rect.contains("y") || !rect.contains("w") || !rect.contains("h")) {
				continue;
			}

			const std::string name = frame["filename"].get<std::string>();
			const float x = rect["x"].get<float>();
			const float y = atlasH - rect["y"].get<float>() - rect["h"].get<float>();
			const float w = rect["w"].get<float>();
			const float h = rect["h"].get<float>();

			sprites[name] = {
				x / atlasW,
				y / atlasH,
				(x + w) / atlasW,
				(y + h) / atlasH
			};
		}

		return sprites;
	}
}

bool Utils::isPointInPolygon(const glm::vec2& point, const std::vector<glm::vec2>& polygon) {
	int intersections = 0;
	size_t count = polygon.size();
	for(size_t i = 0; i < count; ++i) {
		const glm::vec2& v1 = polygon[i];
		const glm::vec2& v2 = polygon[(i + 1) % count];

		if((v1.y > point.y) != (v2.y > point.y)) {
			float slope = (v2.x - v1.x) / (v2.y - v1.y);
			float intersectX = v1.x + slope * (point.y - v1.y);
			if(point.x < intersectX) {
				intersections++;
			}
		}
	}
	return (intersections % 2) == 1;
}

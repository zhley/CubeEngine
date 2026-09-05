#include "misc.h"

#ifdef _WIN32
#include <shlobj.h>
#include <combaseapi.h>
#endif
#ifdef __APPLE__
#include <Foundation/Foundation.h>
#endif

#include <fstream>
#include <string>

#include "Cube/Core/Log.h"
#include "Cube/Utils/Utils.h"
#include "json.hpp"

Cube::Path Utils::getUserConfigDir() {
#ifdef _WIN32
    PWSTR pszPath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &pszPath);
    if (FAILED(hr)) {
        return Cube::Path();
    }
    std::string dir = Cube::Utils::utf16To8(reinterpret_cast<const char16_t*>(pszPath));
    CoTaskMemFree(pszPath);
    return Cube::Path(dir);
#elif defined(__APPLE__)
    @autoreleasepool {
        NSString *home = NSHomeDirectory();
        NSString *appSupport = [home stringByAppendingPathComponent:@"Library/Application Support"];
        return Cube::Path(std::string([appSupport UTF8String]));
    }
#elif defined(__linux__)
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    if (xdg && xdg[0] != '\0') {
        return Cube::Path(xdg);
    }
    const char* home = std::getenv("HOME");
    if (home && home[0] != '\0') {
        return Cube::Path(home) / ".config";
    }
    return Cube::Path();
#else
    #error "Unsupported platform"
#endif
}

nlohmann::json Utils::parseAtlasFile(const Cube::Path& filePath){
    nlohmann::json sprites = nlohmann::json::object();

    std::ifstream file(filePath.fspath());
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

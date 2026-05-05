#pragma once

#include <string>
#include "json.hpp"
#include "glm/glm.hpp"

namespace Utils{
    // 获取用户配置目录
    std::string getUserConfigDir();

    /*
    TexturePacker的json格式示例：
    {
        "frames": [
            {
                "filename": "temperature.png",
                "frame": {"x":808,"y":770,"w":119,"h":252},
                "rotated": false,
                "trimmed": false,
                "spriteSourceSize": {"x":0,"y":0,"w":119,"h":252},
                "sourceSize": {"w":119,"h":252}
            },
            {
                "filename": "ui_room_industrial.png",
                "frame": {"x":2,"y":2,"w":804,"h":852},
                "rotated": false,
                "trimmed": false,
                "spriteSourceSize": {"x":0,"y":0,"w":804,"h":852},
                "sourceSize": {"w":804,"h":852}
            }
        ],
        "meta": {
            "app": "http://www.texturepacker.com",
            "version": "1.0",
            "image": "test.png.png",
            "format": "RGBA8888",
            "size": {"w":1024,"h":2048},
            "scale": "1",
            "smartupdate": "$TexturePacker:SmartUpdate:1aae2ae6b1339b8b158894ea09776aea$"
        }
    }   
    转换成：
    {
        // 转换好的uv坐标 [uvMin.x, uvMin.y, uvMax.x, uvMax.y]
        "temperature.png": [ 0.789, 0.377, 0.906, 0.501],
        "ui_room_industrial.png": [0.002, 0.002, 0.786, 0.416]
    }
    */
    nlohmann::json parseAtlasFile(const std::string& filePath);

    bool isPointInPolygon(const glm::vec2& point, const std::vector<glm::vec2>& polygon);
}

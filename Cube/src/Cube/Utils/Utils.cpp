#include "Utils.h"

#include <fstream>
#include <sstream>

#include "Cube/Core/Log.h"

namespace Cube {

    std::string Utils::readFileToString(const std::string& filePath) {
        std::ifstream file(filePath);
        if(!file.is_open()) {
            CB_CORE_ERROR("cannot open file " + filePath);
            CB_ASSERT("Failed to open file!");
            return "";
        }
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    void Utils::copyFile(const std::string& srcPath, const std::string& destPath) {
        std::ifstream srcFile(srcPath, std::ios::binary);
        if(!srcFile.is_open()) {
            CB_CORE_ERROR("Utils::copyFile: Failed to open source file: {}", srcPath);
            return;
        }

        std::ofstream destFile(destPath, std::ios::binary);
        if(!destFile.is_open()) {
            CB_CORE_ERROR("Utils::copyFile: Failed to open destination file: {}", destPath);
            return;
        }

        destFile << srcFile.rdbuf();

        if(srcFile.fail() || destFile.fail()) {
            CB_CORE_ERROR("Utils::copyFile: Failed to copy file from {} to {}", srcPath, destPath);
        }
    }

    // please pass path with '/' as separator. case-sensitive
    bool Utils::isFileInDirectory(const std::string& file, const std::string& directory) {
        if(directory.empty()) {
            CB_CORE_ERROR("Utils::isFileInDirectory: directory cannot be empty");
            return false;
        }
        if(file.size() <= directory.size()) return false;
        for(int i = 0; i < directory.size(); i++) {
            if(file[i] != directory[i]) {
                return false;
            }
        }
        if(directory.back() != '/' && file[directory.size()] != '/') return false;
        return true;
    }

    // get file name from path
    std::string Utils::getFileName(const std::string& path, bool keepSuffix) {
        size_t begin = path.find_last_of('/');
        size_t end = path.find_last_of('.');
        if(begin == std::string::npos) {
            CB_CORE_ERROR("Utils::getFileName: invalid path");
            return "";
        }
        if(end == std::string::npos) {
            CB_CORE_WARN("Utils::getFileName: No suffix. This may be a directory.");
            return path.substr(begin + 1);
        }
        if(keepSuffix) {
            return path.substr(begin + 1);
        } else {
            return path.substr(begin + 1, end - begin - 1);
        }
    }

    std::vector<uint32_t> Utils::utf8To32(const std::string& utf8_str) {
        std::vector<uint32_t> code_points;
        size_t i = 0;
        const size_t len = utf8_str.size();

        while (i < len) {
            uint8_t c = static_cast<uint8_t>(utf8_str[i]);

            if (c <= 0x7F) { // 1byte U+0000~U+007F
                code_points.push_back(static_cast<uint32_t>(c));
                i += 1;
            } 
            else if (c >= 0xC0 && c <= 0xDF) { // 2bytes U+0080~U+07FF
                if (i + 1 >= len) break; // invalid
                uint8_t c2 = static_cast<uint8_t>(utf8_str[i + 1]);
                uint32_t cp = static_cast<uint32_t>(((c & 0x1F) << 6) | (c2 & 0x3F));
                code_points.push_back(cp);
                i += 2;
            } 
            else if (c >= 0xE0 && c <= 0xEF) { // 3bytes U+0800~U+FFFF
                if (i + 2 >= len) break;
                uint8_t c2 = static_cast<uint8_t>(utf8_str[i + 1]);
                uint8_t c3 = static_cast<uint8_t>(utf8_str[i + 2]);
                uint32_t cp = static_cast<uint32_t>(
                    ((c & 0x0F) << 12) | ((c2 & 0x3F) << 6) | (c3 & 0x3F)
                    );
                code_points.push_back(cp);
                i += 3;
            } 
            else if (c >= 0xF0 && c <= 0xF7) { // 4bytes U+10000~U+10FFFF
                if (i + 3 >= len) break;
                uint8_t c2 = static_cast<uint8_t>(utf8_str[i + 1]);
                uint8_t c3 = static_cast<uint8_t>(utf8_str[i + 2]);
                uint8_t c4 = static_cast<uint8_t>(utf8_str[i + 3]);
                uint32_t cp = static_cast<uint32_t>(
                    ((c & 0x07) << 18) | ((c2 & 0x3F) << 12) | ((c3 & 0x3F) << 6) | (c4 & 0x3F)
                    );
                code_points.push_back(cp);
                i += 4;
            } 
            else {
                // invalid
                i += 1;
            }
        }

        return code_points;
    }
}  // namespace Cube
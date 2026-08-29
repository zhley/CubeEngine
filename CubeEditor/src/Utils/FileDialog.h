#pragma once

#include <windows.h>

#include <string>
#include <vector>

namespace Utils {

class FileDialog {
public:
    struct FilterSpec {
        std::string name;
        std::string spec;
    };

    static std::string openFile(const std::string& title = "select file", const std::vector<FilterSpec>& filters = {}, const std::string& defaultPath = "");
    static std::string saveFile(const std::string& title = "save file", const std::vector<FilterSpec>& filters = {}, const std::string& defaultPath = "", const std::string& defaultExtension = "");
    static std::string selectDir(const std::string& title = "select directory", const std::string& defaultPath = "");
    static std::vector<std::string> openMultiFiles(const std::string& title = "select multiple files", const std::vector<FilterSpec>& filters = {}, const std::string& defaultPath = "");
};

}

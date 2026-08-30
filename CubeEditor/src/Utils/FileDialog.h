#pragma once

#include <windows.h>

#include <string>
#include <vector>

#include "Cube/Core/Path.h"

namespace Utils {

class FileDialog {
public:
    struct FilterSpec {
        std::string name;
        std::string spec;
    };

    static Cube::Path openFile(const std::string& title = "select file", const std::vector<FilterSpec>& filters = {}, const Cube::Path& defaultPath = Cube::Path());
    static Cube::Path saveFile(const std::string& title = "save file", const std::vector<FilterSpec>& filters = {}, const Cube::Path& defaultPath = Cube::Path(), const std::string& defaultExtension = "");
    static Cube::Path selectDir(const std::string& title = "select directory", const Cube::Path& defaultPath = Cube::Path());
    static std::vector<Cube::Path> openMultiFiles(const std::string& title = "select multiple files", const std::vector<FilterSpec>& filters = {}, const Cube::Path& defaultPath = Cube::Path());
};

}

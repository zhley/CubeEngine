#include "Script.h"

#include "Cube/Script/ScriptRuntime.h"

namespace Cube {

Script::Script(const std::string& filePath) {
    module = ScriptRuntime::parseModule(filePath);
}

}

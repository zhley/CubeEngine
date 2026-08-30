#include "Script.h"

#include "Cube/Script/ScriptRuntime.h"

namespace Cube {

Script::Script(const Cube::Path& filePath) {
    module = ScriptRuntime::parseModule(filePath);
}

}

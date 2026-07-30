#include "Engine.h"

#include "TypeRegister.h"

namespace Cube {

    void Engine::init() {
        Log::init();
        registerTypes();
    }

}  // namespace Cube
#include "Context.h"

namespace Cube {

    Context::Context() {}

    Context::~Context() {
        delete defaultShader;
        delete whiteTex;
    }

}  // namespace Cube
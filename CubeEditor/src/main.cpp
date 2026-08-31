#include "Cube/Core/Log.h"
#include "Cube/Utils/Utils.h"

#include "App/EditorApp.h"
#include "App/GuidancePage.h"

int main() {
    Cube::Utils::setConsoleUtf8();
    CB_ASSERT(GetACP() == 65001);
    
    EditorApp::init();
    auto app = EditorApp::get();
    app->switchPage(new GuidancePage);
    app->run();
    EditorApp::shutdown();
    return 0;
}

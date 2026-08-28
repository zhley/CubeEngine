#include "App/EditorApp.h"
#include "App/GuidancePage.h"
#include "Cube/Core/Engine.h"

int main() {
    Cube::Engine::init();
    Cube::Engine::setApp(new EditorApp({1920, 1080, "Cube Editor"}));
    EditorApp* app = static_cast<EditorApp*>(Cube::Engine::getApp());
    app->switchPage(new GuidancePage);
    app->run();
    Cube::Engine::shutdown();
    return 0;
}

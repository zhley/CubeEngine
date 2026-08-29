#include "Cube/Core/Engine.h"
#include "Cube/Utils/Utils.h"

#include "App/EditorApp.h"
#include "App/GuidancePage.h"

int main() {
    Cube::Utils::setConsoleUtf8();

    UINT acp = GetACP(); // 获取当前进程的 ANSI 代码页
    std::cout << "Current ANSI Code Page: " << acp << std::endl;
    
    Cube::Engine::init();
    Cube::Engine::setApp(new EditorApp({1920, 1080, "Cube Editor"}));
    EditorApp* app = static_cast<EditorApp*>(Cube::Engine::getApp());
    app->switchPage(new GuidancePage);
    app->run();
    Cube::Engine::shutdown();
    return 0;
}

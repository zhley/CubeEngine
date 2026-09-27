#include <Windows.h>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "zeta/compiler/bytecode.h"

#include "Cube/Core/Application.h"
#include "Cube/Core/Engine.h"
#include "Cube/Core/Log.h"
#include "Cube/Core/Path.h"
#include "Cube/Core/Window.h"
#include "Cube/Resource/ResourceManager.h"
#include "Cube/Resource/ResPtr.h"
#include "Cube/Resource/NodeTree.h"
#include "Cube/Scene/Node.h"
#include "Cube/Script/ScriptRuntime.h"
#include "Cube/Utils/Utils.h"

namespace {

// TODO: 以下配置将来改为从配置文件读取
constexpr int kWindowWidth = 1280;
constexpr int kWindowHeight = 720;
constexpr const char* kWindowTitle = "Cube Game";
constexpr const char* kEngineScriptSearchPath = "D:/mycode/vsProject/CubeEngine/Cube/scripts";
constexpr const char* kControllerScriptName = "main.zt";
constexpr const char* kAssetMapFileName = "asset.json";

enum class ParseResult {
    Continue,
    ExitSuccess,
    ExitFailure
};

void printUsage(const char* programName) {
    std::printf("Usage: %s [options]\n", programName);
    std::printf("Options:\n");
    std::printf("  -n, --node <identifier>  Resource identifier of the initial node tree\n");
    std::printf("  -h, --help               Show this help message\n");
}

ParseResult parseArgs(int argc, char* argv[], std::string& nodeIdentifier) {
    for(int i = 1; i < argc; ++i) {
        const std::string_view arg(argv[i]);
        if(arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return ParseResult::ExitSuccess;
        }
        // "-n value" and "-n=value" are both valid
        std::string_view option;
        std::string_view inlineValue;
        if(const std::size_t eq = arg.find('='); eq != std::string_view::npos) {
            option = arg.substr(0, eq);
            inlineValue = arg.substr(eq + 1);
        } else {
            option = arg;
        }
        if(option != "-n" && option != "--node") {
            CB_ERROR("Unknown option: {}", arg);
            printUsage(argv[0]);
            return ParseResult::ExitFailure;
        }
        std::string value(inlineValue);
        if(value.empty()) {
            if(i + 1 >= argc) {
                CB_ERROR("Option '{}' requires a value", option);
                printUsage(argv[0]);
                return ParseResult::ExitFailure;
            }
            value = argv[++i];
        }
        nodeIdentifier = value;
    }
    return ParseResult::Continue;
}

Cube::Path getExecutableDir() {
    // TODO: MAX_PATH 对超长路径不够, 将来改为循环扩容的动态缓冲
    wchar_t buffer[MAX_PATH];
    const DWORD length = GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    if(length == 0 || length >= MAX_PATH) {
        return Cube::Path();
    }
    const std::u16string_view utf16(reinterpret_cast<const char16_t*>(buffer), length);
    Cube::Path exePath(Cube::Utils::utf16To8(utf16));
    exePath.removeFilename();
    return exePath;
}

class ScriptGameController : public Cube::IGameController {
public:
    explicit ScriptGameController(Cube::Path scriptPath) : scriptPath(std::move(scriptPath)) {}

    void init() override {
        Cube::Application* app = Cube::Engine::getApp();
        Zeta::VM& vm = app->getScriptRuntime().getVM();
        std::unique_ptr<Zeta::Module> module = Cube::ScriptRuntime::parseModule(scriptPath);
        if (!module) {
            CB_ERROR("ScriptGameController::init(): failed to parse script {}", scriptPath);
            return;
        }
        vm.loadModule(module.get());
        initIndex = vm.findGlobal(module->name, "init");
        updateIndex = vm.findGlobal(module->name, "update");
        ready = true;
        if (initIndex >= 0) {
            vm.push(vm.getGlobal(initIndex));
            vm.call(0);
            vm.pop();
        }
        CB_INFO("ScriptGameController ready: {}", scriptPath);
    }

    void update(float deltaTime) override {
        if (!ready) {
            return;
        }
        if (updateIndex >= 0) {
            Zeta::VM& vm = Cube::Engine::getApp()->getScriptRuntime().getVM();
            vm.push(Zeta::Value(static_cast<double>(deltaTime)));
            vm.push(vm.getGlobal(updateIndex));
            vm.call(1);
            vm.pop();
        }
    }

private:
    Cube::Path scriptPath;
    int initIndex = -1;
    int updateIndex = -1;
    bool ready = false;
};

}  // namespace

int main(int argc, char* argv[]) {
    Cube::Utils::setConsoleUtf8();
    
    Cube::Engine::init();

    std::string nodeIdentifier;
    switch(parseArgs(argc, argv, nodeIdentifier)) {
        case ParseResult::ExitFailure: return 1;
        case ParseResult::ExitSuccess: return 0;
        case ParseResult::Continue: break;
    }

    const Cube::Path exeDir = getExecutableDir();
    if(exeDir.empty()) {
        CB_ERROR("Failed to locate the executable directory");
        return 1;
    }
    CB_INFO("Executable directory: {}", exeDir);

    const Cube::Path controllerPath = exeDir / kControllerScriptName;
    std::unique_ptr<Cube::IGameController> controller;
    if(std::filesystem::exists(controllerPath.fspath())) {
        controller = std::make_unique<ScriptGameController>(controllerPath);
    } else {
        CB_INFO("Controller script not found, running without a game controller: {}", controllerPath);
    }

    Cube::Engine::setApp(new Cube::Application({kWindowWidth, kWindowHeight, kWindowTitle}, {kEngineScriptSearchPath}, std::move(controller)));
    Cube::Application* app = Cube::Engine::getApp();

    const Cube::Path assetMapPath = exeDir / kAssetMapFileName;
    if(std::filesystem::exists(assetMapPath.fspath())) {
        app->getResourceManager().init(assetMapPath);
        CB_INFO("Asset map loaded: {}", assetMapPath);
    } else {
        CB_INFO("Asset map not found, skip resource initialization: {}", assetMapPath);
    }

    if(nodeIdentifier.empty()) {
        CB_INFO("No initial node specified, use -n/--node <identifier> to load one");
    } else {
        // The initial node tree is a resource blueprint: load by identifier
        // (mapped in asset.json) and attach the instantiated subtree to the root.
        app->getRootNode()->addChildFrom(Cube::ResPtr<Cube::NodeTree>(nodeIdentifier));
        CB_INFO("Initial node tree loaded: {}", nodeIdentifier);
    }

    app->run();
    Cube::Engine::shutdown();
    return 0;
}

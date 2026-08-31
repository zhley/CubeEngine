#pragma once

#include "Cube/Core/Path.h"
#include "Cube/Core/Window.h"
#include "Cube/Core/Application.h"
#include "Cube/Core/Engine.h"

#include "Page.h"

class EditorApp : public Cube::Application {
public:
    void switchPage(Page* page);
    void run() override;

    static Cube::Path getConfigDir();

    static void init() {
        Cube::Engine::init();
        Cube::Engine::setApp(new EditorApp({1920, 1080, "Cube Editor"}));
    }
    static EditorApp* get() { return static_cast<EditorApp*>(Cube::Engine::getApp()); }
    static void shutdown() { Cube::Engine::shutdown(); }

private:
    EditorApp(const Cube::WindowPros& windowPros);
    ~EditorApp();

    std::unique_ptr<Page> currentPage;

    void imGuiInit();
    void setDarkTheme();
};

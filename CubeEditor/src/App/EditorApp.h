#pragma once

#include "Cube/Core/Window.h"
#include "Cube/Core/Application.h"

#include "Page.h"

class EditorApp : public Cube::Application {
public:
    static const std::string userConfigDir;

    EditorApp(const Cube::WindowPros& windowPros);
    ~EditorApp();

    void switchPage(Page* page);
    void run() override;

    // global
    std::vector<std::string> projectsPathCache;

private:
    std::unique_ptr<Page> currentPage;

    void imGuiInit();
    void loadConfig();
    void saveConfig();
    void setDarkTheme();
};
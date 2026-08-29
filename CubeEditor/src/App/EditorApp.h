#pragma once

#include <filesystem>

#include "Cube/Core/Window.h"
#include "Cube/Core/Application.h"

#include "Page.h"

class EditorApp : public Cube::Application {
public:
    EditorApp(const Cube::WindowPros& windowPros);
    ~EditorApp();

    void switchPage(Page* page);
    void run() override;

    static std::filesystem::path getConfigDir();

private:
    std::unique_ptr<Page> currentPage;

    void imGuiInit();
    void setDarkTheme();
};
#pragma once

#include "KeyCodes.h"
#include "MouseCodes.h"

#include <string>
#include <glad/glad.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <Windows.h>

#include "Cube/Event/Event.h"
#include "Cube/Renderer/Context.h"

namespace Cube {

    struct WindowPros {
        int width = 800;
        int height = 600;
        std::string title = "Cube Engine";
        WindowPros(int width, int height, const std::string& title) : width(width), height(height), title(title) {}
    };

    struct MousePos {
        double x;
        double y;
    };

    class Window {
    public:
        Window(const WindowPros& pros, EventDispatcher* eventDispatcher, GLFWwindow* shareContext = nullptr);
        virtual ~Window();
        void init(GLFWwindow* shareContext);
        const WindowPros& getPros() const;
        GLFWwindow* getNativeWindow() const;

        HWND getWin32Window();

        void update();
        void makeContext() const;
        void close();

        // input
        bool isKeyPressed(KeyCode keyCode);
        bool isMouseButtonPressed(MouseCode mouseCode);
        MousePos getMousePosition();

    private:
        static void windowErrorCallBack(int error_code, const char* description);

        static int windowCnt;
        WindowPros pros;
        GLFWwindow* window;
        Context* context = nullptr;
        EventDispatcher* eventDispatcher;
    };
}  // namespace Cube

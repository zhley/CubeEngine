#include "pch.h"
#include "Window.h"

#include <iostream>

#include "Cube/Event/ApplicationEvent.h"
#include "Cube/Event/KeyEvent.h"
#include "Cube/Event/MouseEvent.h"
#include "Timer.h"
#include "Cube/Renderer/Renderer.h"

#include <GLFW/glfw3native.h>

namespace Cube {

    int Window::windowCnt = 0;

    Window::Window(const WindowPros& pros, EventDispatcher* eventDispatcher, GLFWwindow* shareContext) : pros(pros), eventDispatcher(eventDispatcher) { 
        init(shareContext); 
        ++windowCnt;
    }

    Window::~Window() {
        glfwMakeContextCurrent(window);
        if(Renderer2D::currentContext == context) {
            Renderer2D::currentContext = nullptr;
        }
        delete context;

        glfwDestroyWindow(window);
        --windowCnt;
        if(!windowCnt) {
            glfwTerminate();
        }
    }

    void Window::init(GLFWwindow* shareContext) {
        if(!windowCnt) {
            if(!glfwInit()) {
                CB_CORE_ERROR("glfwInit failed!");
            }
            CB_CORE_INFO("glfw initialize");
        }
        glfwWindowHint(GLFW_ALPHA_BITS, 8); // 8-bits Alpha
        window = glfwCreateWindow(pros.width, pros.height, pros.title.c_str(), nullptr, shareContext);

        glfwSetWindowUserPointer(window, this);

        glfwMakeContextCurrent(window);
        context = new Context();
        Renderer2D::currentContext = context;
        Renderer2D::init();

        Renderer2D::setViewport(pros.width, pros.height);

        glfwSetErrorCallback(windowErrorCallBack);

        glfwSetWindowCloseCallback(window, [](GLFWwindow* w) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(w));
            window->eventDispatcher->dispatch(WindowCloseEvent(window));
        });

        glfwSetWindowSizeCallback(window, [](GLFWwindow* w, int width, int height) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(w));

            window->pros.width = width;
            window->pros.height = height;

            window->eventDispatcher->dispatch(WindowResizeEvent(window, width, height));
        });

        glfwSetKeyCallback(window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(w));
            switch(action) {
            case GLFW_PRESS:
                window->eventDispatcher->dispatch(KeyPressedEvent(key, false));
                break;
            case GLFW_REPEAT:
                window->eventDispatcher->dispatch(KeyPressedEvent(key, true));
                break;
            case GLFW_RELEASE:
                window->eventDispatcher->dispatch(KeyReleasedEvent(key));
                break;
            }
        });

        glfwSetMouseButtonCallback(window, [](GLFWwindow* w, int button, int action, int mods) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(w));
            double xPos, yPos;
            glfwGetCursorPos(w, &xPos, &yPos);
            switch(action) {
            case GLFW_PRESS:
                window->eventDispatcher->dispatch(MousePressedEvent(xPos, yPos, button));
                break;
            case GLFW_RELEASE:
                window->eventDispatcher->dispatch(MouseReleasedEvent(xPos, yPos, button));
                break;
            }
        });

        glfwSetScrollCallback(window, [](GLFWwindow* w, double xoffset, double yoffset) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(w));
            double xPos, yPos;
            glfwGetCursorPos(w, &xPos, &yPos);
            window->eventDispatcher->dispatch(MouseScrolledEvent(xPos, yPos, xoffset, yoffset));
        });

        glfwSetCursorPosCallback(window, [](GLFWwindow* w, double xpos, double ypos) {
            Window* window = static_cast<Window*>(glfwGetWindowUserPointer(w));
            window->eventDispatcher->dispatch(MouseMovedEvent(xpos, ypos));
        });
    }

    const WindowPros& Window::getPros() const { return pros; }

    GLFWwindow* Window::getNativeWindow() const { return window; }

    HWND Window::getWin32Window() {
        return glfwGetWin32Window(window);
    }

    void Window::update() {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    void Window::makeContext() const {
        glfwMakeContextCurrent(window);
        Renderer2D::currentContext = context;
    }

    void Window::close() {
        eventDispatcher->dispatch(WindowCloseEvent(this));
    }

    bool Window::isKeyPressed(KeyCode keyCode) {
        return glfwGetKey(window, keyCode) == GLFW_PRESS;
    }

    bool Window::isMouseButtonPressed(MouseCode mouseCode) {
        return glfwGetMouseButton(window, mouseCode) == GLFW_PRESS;
    }

    MousePos Window::getMousePosition() {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        return {x, y};
    }

    void Window::windowErrorCallBack(int error_code, const char* description) {
        CB_CORE_ERROR("glfwWindowError: {}", description);
    }
}  // namespace Cube

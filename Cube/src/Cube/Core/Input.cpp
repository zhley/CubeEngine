#include "Input.h"

#include "GLFW/glfw3.h"

namespace Cube {

    bool Input::isMouseButtonPressed(MouseCode code) {
        GLFWwindow* win = glfwGetCurrentContext();
        return glfwGetMouseButton(win, code) == GLFW_PRESS;
    }

    bool Input::isKeyPressed(KeyCode code) {
        GLFWwindow* win = glfwGetCurrentContext();
        return glfwGetKey(win, code) == GLFW_PRESS;
    }

    glm::vec2 Input::getMousePos() {
        double x, y;
        GLFWwindow* win = glfwGetCurrentContext();
        glfwGetCursorPos(win, &x, &y);
        return {x, y};
    }

    float Input::getMousePosX() {
        double x, y;
        GLFWwindow* win = glfwGetCurrentContext();
        glfwGetCursorPos(win, &x, &y);
        return x;
    }

    float Input::getMousePosY() {
        double x, y;
        GLFWwindow* win = glfwGetCurrentContext();
        glfwGetCursorPos(win, &x, &y);
        return y;
    }

}  // namespace Cube
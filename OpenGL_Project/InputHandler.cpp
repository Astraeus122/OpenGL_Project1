#include "InputHandler.h"
#include <iostream>

InputHandler::InputHandler(GLFWwindow* window) : window(window), wireframeMode(false), cursorVisible(true), lastCursorPrintTime(0.0) {}

void InputHandler::processInput(float deltaTime) {
    static bool wireframeTogglePressed = false;

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
        if (!wireframeTogglePressed) {
            toggleWireframeMode();
            wireframeTogglePressed = true;
        }
    }
    else {
        wireframeTogglePressed = false;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        printCursorCoordinates();
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        toggleCursorVisibility();
    }
}

void InputHandler::toggleWireframeMode() {
    wireframeMode = !wireframeMode;
    if (wireframeMode) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void InputHandler::printCursorCoordinates() {
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    std::cout << "Cursor Position: (" << xpos << ", " << ypos << ")" << std::endl;
}

void InputHandler::toggleCursorVisibility() {
    cursorVisible = !cursorVisible;
    if (cursorVisible) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
}

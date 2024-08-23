#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

#include <glfw3.h>

class InputHandler {
public:
    InputHandler(GLFWwindow* window);
    void processInput(float deltaTime);

private:
    GLFWwindow* window;
    bool wireframeMode;
    bool cursorVisible;
    double lastCursorPrintTime;

    void toggleWireframeMode();
    void printCursorCoordinates();
    void toggleCursorVisibility();
};

#endif 

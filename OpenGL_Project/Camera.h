#ifndef CAMERA_H
#define CAMERA_H

#include <glfw3.h>
#include "Dependencies/glm/glm.hpp"

class Camera {
public:
    Camera(GLFWwindow* window, glm::vec3 position, float radius);
    void processInput(float deltaTime);
    void processMouseMovement(float xoffset, float yoffset);
    void processMouseScroll(float yoffset);
    void update(float deltaTime);
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetProjectionMatrix() const;

    glm::vec3 getPosition() const { return position; }

    GLFWwindow* window;
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    float yaw;
    float pitch;
    float fov;
    float nearClip;
    float farClip;
    float lastX;
    float lastY;
    bool firstMouse;
    float sensitivity;
    float speed; 
    float radius;
    bool automaticMovement;


    void updateCameraVectors();
};

#endif 

#include "Camera.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

Camera::Camera(GLFWwindow* window, glm::vec3 position, float radius)
    : window(window), position(position), radius(radius), yaw(-90.0f), pitch(0.0f), automaticMovement(false), fov(45.0f), nearClip(0.1f), farClip(5000.0f) {
    lastX = 400.0f; // Assuming the window width is 800
    lastY = 300.0f; // Assuming the window height is 600
    firstMouse = true;
    sensitivity = 0.1f; 
    front = glm::vec3(0.0f, 0.0f, -1.0f); 
    updateCameraVectors();
}

void Camera::processInput(float deltaTime) {
    float speed = 2.5f * deltaTime; // Adjusted base speed for movement

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        speed *= 2.0f; // Increase speed when Left Shift is pressed
    }

    // Camera position movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        position += front * speed;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        position -= front * speed;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        position -= right * speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        position += right * speed;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        position -= up * speed;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        position += up * speed;
}

void Camera::processMouseMovement(float xoffset, float yoffset) {
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Constrain the pitch
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    updateCameraVectors();
}


void Camera::processMouseScroll(float yoffset) {
    if (fov >= 1.0f && fov <= 45.0f)
        fov -= yoffset;
    if (fov <= 1.0f)
        fov = 1.0f;
    if (fov >= 45.0f)
        fov = 45.0f;
}

void Camera::update(float deltaTime) {
    if (automaticMovement) {
        yaw += 20.0f * deltaTime; 
    }
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}


glm::mat4 Camera::getProjectionMatrix() const {
    return glm::perspective(glm::radians(fov), 16.0f / 9.0f, nearClip, farClip);
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->front = glm::normalize(front);

 
    right = glm::normalize(glm::cross(this->front, glm::vec3(0.0f, 1.0f, 0.0f)));
    up = glm::normalize(glm::cross(right, this->front));
}

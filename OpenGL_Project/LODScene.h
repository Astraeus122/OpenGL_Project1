#pragma once

#include "ShaderLoader.h"
#include "Camera.h"
#include "Plane.h" // Or your terrain object class

class LODScene {
public:
    LODScene(ShaderLoader& shaderLoader, Camera& camera);

    void initialize();
    void render();
    void update(float deltaTime);

private:
    GLuint shaderProgram;
    GLuint tessControlShader;
    GLuint tessEvalShader;
    GLuint vertexShader;
    GLuint fragmentShader;

    Camera& camera;
    Plane terrain;  // You can replace this with your custom terrain object

    glm::mat4 modelMatrix;
};

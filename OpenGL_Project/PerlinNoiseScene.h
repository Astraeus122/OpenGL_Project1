#pragma once

#include "ShaderLoader.h"
#include "Camera.h"
#include "Skybox.h"
#include "PerlinNoise.h"
#include <glew.h>
#include <glfw3.h>
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

class PerlinNoiseScene {
public:
    PerlinNoiseScene(ShaderLoader& shaderLoader, Camera& camera, Skybox& skybox);
    void render();
    void initialize();

private:
    ShaderLoader& shaderLoader;
    Camera& camera;
    Skybox& skybox;
    PerlinNoise perlinNoise;

    GLuint perlinShader;
    GLuint noiseTexture;
    GLuint quadVAO, quadVBO;
    glm::mat4 quadModelMatrix;

    void generateNoiseTexture();
    void setupQuad();
    void checkGLError(const char* operation);
};

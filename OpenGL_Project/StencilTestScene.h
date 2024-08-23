#pragma once

#include "ShaderLoader.h"
#include "Camera.h"
#include "Skybox.h"
#include "InstancedRenderer.h"

class StencilTestScene {
public:
    StencilTestScene(ShaderLoader& shaderLoader, Camera& camera, Skybox& skybox, InstancedRenderer& renderer);
    void render();

private:
    ShaderLoader& shaderLoader;
    Camera& camera;
    Skybox& skybox;
    InstancedRenderer& renderer;
    GLuint shaderProgram;
    GLuint outlineShaderProgram;
};


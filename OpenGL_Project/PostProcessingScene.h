#pragma once

#include "ShaderLoader.h"
#include "Camera.h"
#include "InstancedRenderer.h"

class PostProcessingScene {
public:
    PostProcessingScene(ShaderLoader& shaderLoader, Camera& camera, InstancedRenderer& renderer, GLuint framebuffer, GLuint textureColorbuffer);
    void render(int currentEffect, GLuint quadVAO);

private:
    ShaderLoader& shaderLoader;
    Camera& camera;
    InstancedRenderer& renderer;
    GLuint framebuffer;
    GLuint textureColorbuffer;
    GLuint inversionShader;
    GLuint greyscaleShader;
    GLuint rainingShader;
    GLuint additionalShader;

    void renderQuad(GLuint quadVAO);
};

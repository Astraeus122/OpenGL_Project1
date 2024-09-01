#pragma once

#include "ShaderLoader.h"
#include "Camera.h"
#include "Skybox.h" 
#include "InstancedRenderer.h"
#include <glew.h>
#include <vector>

class PostProcessingScene {
public:
    PostProcessingScene(ShaderLoader& shaderLoader, Camera& camera, Skybox& skybox, InstancedRenderer& renderer, unsigned int width, unsigned int height);
    void render(int currentEffect);
    void nextEffect();

private:
    ShaderLoader& shaderLoader;
    Camera& camera;
    Skybox& skybox;
    InstancedRenderer& renderer;

    GLuint framebuffer;
    GLuint textureColorbuffer;
    GLuint rbo;

    GLuint quadVAO, quadVBO;
    std::vector<GLuint> effectShaders;
    int currentEffect;

    GLuint shaderProgram;  // Base shader program used for rendering the scene

    void setupFramebuffer(unsigned int width, unsigned int height);
    void setupQuad();
    void loadShaders();
    void applyEffect(GLuint textureId, int effectId);
};

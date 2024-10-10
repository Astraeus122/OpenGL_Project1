#ifndef SHADOWSCENE_H
#define SHADOWSCENE_H

#include "ShaderLoader.h"
#include "Camera.h"
#include "Skybox.h"
#include "InstancedRenderer.h"
#include "LightManager.h"
#include <glew.h>

class ShadowScene {
public:
    ShadowScene(ShaderLoader& shaderLoader, Camera& camera, Skybox& skybox, InstancedRenderer& renderer, LightManager& lightManager);
    void initialize();
    void render();

private:
    ShaderLoader& shaderLoader;
    Camera& camera;
    Skybox& skybox;
    InstancedRenderer& renderer;
    LightManager& lightManager;

    GLuint shadowFramebuffer;
    GLuint shadowDepthTexture;

    GLuint shadowShaderProgram;
    GLuint lightingShaderProgram;

    glm::mat4 lightSpaceMatrix;

    void setupShadowMap();
    void renderShadowPass();
    void renderSceneWithShadows();
};

#endif // SHADOWSCENE_H

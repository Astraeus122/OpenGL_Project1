#include "ShadowScene.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

ShadowScene::ShadowScene(ShaderLoader& shaderLoader, Camera& camera, Skybox& skybox, InstancedRenderer& renderer, LightManager& lightManager)
    : shaderLoader(shaderLoader), camera(camera), skybox(skybox), renderer(renderer), lightManager(lightManager) {
    shadowShaderProgram = shaderLoader.CreateProgram("Resources/Shaders/shadow_vertex_shader.vert", "Resources/Shaders/shadow_fragment_shader.frag");
    lightingShaderProgram = shaderLoader.CreateProgram("Resources/Shaders/lighting_vertex_shader.vert", "Resources/Shaders/lighting_fragment_shader.frag");
}

void ShadowScene::initialize() {
    setupShadowMap();
}

void ShadowScene::setupShadowMap() {
    glGenFramebuffers(1, &shadowFramebuffer);
    glGenTextures(1, &shadowDepthTexture);

    glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowScene::render() {
    renderShadowPass();
    renderSceneWithShadows();
}

void ShadowScene::renderShadowPass() {
    glUseProgram(shadowShaderProgram);

    // Set up the light's perspective
    glm::vec3 lightPos = glm::vec3(150.0f, 300.0f, 150.0f);
    glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 1.0f, 300.0f);
    glm::mat4 lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    lightSpaceMatrix = lightProjection * lightView;
    glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    glViewport(0, 0, 1024, 1024);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderer.render(shadowShaderProgram, glm::mat4(1.0f));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowScene::renderSceneWithShadows() {
    glUseProgram(lightingShaderProgram);

    // Pass light space matrix for shadow calculations
    glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    // Bind the shadow map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTexture);
    glUniform1i(glGetUniformLocation(lightingShaderProgram, "shadowMap"), 1);

    // Render scene objects with shadows
    renderer.render(lightingShaderProgram, glm::mat4(1.0f));
}

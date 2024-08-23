#include "PostProcessingScene.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

PostProcessingScene::PostProcessingScene(ShaderLoader& shaderLoader, Camera& camera, InstancedRenderer& renderer, GLuint framebuffer, GLuint textureColorbuffer)
    : shaderLoader(shaderLoader), camera(camera), renderer(renderer), framebuffer(framebuffer), textureColorbuffer(textureColorbuffer) {
    inversionShader = shaderLoader.CreateProgram("Resources/Shaders/inversion.vert", "Resources/Shaders/inversion.frag");
    greyscaleShader = shaderLoader.CreateProgram("Resources/Shaders/greyscale.vert", "Resources/Shaders/greyscale.frag");
    rainingShader = shaderLoader.CreateProgram("Resources/Shaders/raining.vert", "Resources/Shaders/raining.frag");
    additionalShader = shaderLoader.CreateProgram("Resources/Shaders/additional.vert", "Resources/Shaders/additional.frag");
}

void PostProcessingScene::render(int currentEffect, GLuint quadVAO) {
    // Bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 viewProjection = camera.getProjectionMatrix() * camera.getViewMatrix();
    renderer.render(inversionShader, viewProjection);

    // Unbind framebuffer and render to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint postProcessingShader = 0;
    switch (currentEffect) {
    case 1: postProcessingShader = inversionShader; break;
    case 2: postProcessingShader = greyscaleShader; break;
    case 3: postProcessingShader = rainingShader; break;
    default: break;
    }

    if (postProcessingShader != 0) {
        glUseProgram(postProcessingShader);
    }

    renderQuad(quadVAO);
}

void PostProcessingScene::renderQuad(GLuint quadVAO) {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

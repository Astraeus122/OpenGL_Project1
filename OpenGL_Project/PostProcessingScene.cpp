#include "PostProcessingScene.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include <iostream>
#include "string"

PostProcessingScene::PostProcessingScene(ShaderLoader& shaderLoader, Camera& camera, InstancedRenderer& renderer, GLuint framebuffer, GLuint textureColorbuffer)
    : shaderLoader(shaderLoader), camera(camera), renderer(renderer), framebuffer(framebuffer), textureColorbuffer(textureColorbuffer) {
    // Load shaders with a common vertex shader
    inversionShader = shaderLoader.CreateProgram("Resources/Shaders/post_processing.vert", "Resources/Shaders/inversion.frag");
    greyscaleShader = shaderLoader.CreateProgram("Resources/Shaders/post_processing.vert", "Resources/Shaders/greyscale.frag");
    rainingShader = shaderLoader.CreateProgram("Resources/Shaders/post_processing.vert", "Resources/Shaders/raining.frag");
}

void PostProcessingScene::render(int currentEffect, GLuint quadVAO) {
    // Bind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST); // Enable depth testing when rendering scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 viewProjection = camera.getProjectionMatrix() * camera.getViewMatrix();
    renderer.render(inversionShader, viewProjection); // Render the scene normally

    // Unbind framebuffer and render to screen
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // Disable depth testing for screen quad rendering
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint postProcessingShader = 0;
    switch (currentEffect) {
    case 1: postProcessingShader = inversionShader; break;
    case 2: postProcessingShader = greyscaleShader; break;
    case 3: postProcessingShader = rainingShader; break;
    default: break;
    }

    if (postProcessingShader != 0) {
        glUseProgram(postProcessingShader);
        glUniform1f(glGetUniformLocation(postProcessingShader, "time"), static_cast<float>(glfwGetTime())); // Pass time uniform for effects
        glActiveTexture(GL_TEXTURE0); // Ensure texture unit 0 is active
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer); // Bind the framebuffer texture
        glUniform1i(glGetUniformLocation(postProcessingShader, "screenTexture"), 0); // Set the texture uniform to use texture unit 0

        renderQuad(quadVAO);  // Render the screen quad
    }

    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "OpenGL Error: " << error << std::endl;
    }
}


void PostProcessingScene::renderQuad(GLuint quadVAO) {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
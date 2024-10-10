#include "PostProcessingScene.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include <iostream>
#include <string>

PostProcessingScene::PostProcessingScene(ShaderLoader& shaderLoader, Camera& camera, Skybox& skybox,
    InstancedRenderer& mineRenderer, InstancedRenderer& cannonRenderer,
    InstancedRenderer& alienRenderer, unsigned int width, unsigned int height)
    : shaderLoader(shaderLoader), camera(camera), skybox(skybox),
    mineRenderer(mineRenderer), cannonRenderer(cannonRenderer), alienRenderer(alienRenderer), currentEffect(0) {

    // Load base scene shaders
    shaderProgram = shaderLoader.CreateProgram("Resources/Shaders/vertex_shader.vert", "Resources/Shaders/fragment_shader.frag");

    // Load post-processing shaders
    loadShaders();

    // Set up framebuffer and quad for post-processing
    setupFramebuffer(width, height);
    setupQuad();
}

void PostProcessingScene::setupFramebuffer(unsigned int width, unsigned int height) {
    // Generate framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Generate texture to hold framebuffer content
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // Generate renderbuffer for depth and stencil
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PostProcessingScene::setupQuad() {
    float quadVertices[] = {
        // First Triangle
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

         // Second Triangle
         -1.0f,  1.0f,  0.0f, 1.0f,
          1.0f, -1.0f,  1.0f, 0.0f,
          1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadIndices[] = {
        0, 1, 2, 0, 2, 3
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void PostProcessingScene::loadShaders() {
    effectShaders.push_back(shaderLoader.CreateProgram("Resources/Shaders/post_processing.vert", "Resources/Shaders/inversion.frag"));
    effectShaders.push_back(shaderLoader.CreateProgram("Resources/Shaders/post_processing.vert", "Resources/Shaders/greyscale.frag"));
    effectShaders.push_back(shaderLoader.CreateProgram("Resources/Shaders/post_processing.vert", "Resources/Shaders/raining.frag"));
    effectShaders.push_back(shaderLoader.CreateProgram("Resources/Shaders/post_processing.vert", "Resources/Shaders/sobel.frag"));
}

void PostProcessingScene::render(int currentEffect) {
    // Render the scene to the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glEnable(GL_DEPTH_TEST); // Enable depth testing for the scene
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render the skybox first
    glDepthMask(GL_FALSE); // Disable writing to depth buffer
    glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix())); // Remove translation from the view matrix
    skybox.render(camera.getProjectionMatrix() * view);
    glDepthMask(GL_TRUE); // Enable depth buffer writing

    // Set up shader program
    glUseProgram(shaderProgram);

    // Render the mine
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    float rotationAngle = glm::radians(glfwGetTime() * 20.0f);
    modelMatrix = glm::rotate(modelMatrix, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);
    mineRenderer.render(shaderProgram, modelMatrix);

    // Render the cannon
    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(20.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    cannonRenderer.render(shaderProgram, modelMatrix);

    // Render the alien
    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-20.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    alienRenderer.render(shaderProgram, modelMatrix);

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST); // Disable depth testing for screen quad rendering
    glClear(GL_COLOR_BUFFER_BIT); // Clear the screen to prepare for post-processing effects

    // Apply post-processing effects
    applyEffect(textureColorbuffer, currentEffect);
}


void PostProcessingScene::applyEffect(GLuint textureId, int effectId) {
    glBindVertexArray(quadVAO);
    glUseProgram(effectShaders[effectId]);
    glUniform1i(glGetUniformLocation(effectShaders[effectId], "screenTexture"), 0);
    glUniform1f(glGetUniformLocation(effectShaders[effectId], "time"), static_cast<float>(glfwGetTime()));
    glUniform2f(glGetUniformLocation(effectShaders[effectId], "resolution"), 800.0f, 600.0f);  // Replace with actual width and height if different
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void PostProcessingScene::nextEffect() {
    currentEffect = (currentEffect + 1) % effectShaders.size();
}

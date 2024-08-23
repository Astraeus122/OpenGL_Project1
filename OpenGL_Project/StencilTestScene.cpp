#include "StencilTestScene.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

StencilTestScene::StencilTestScene(ShaderLoader& shaderLoader, Camera& camera, Skybox& skybox, InstancedRenderer& renderer)
    : shaderLoader(shaderLoader), camera(camera), skybox(skybox), renderer(renderer) {
    shaderProgram = shaderLoader.CreateProgram("Resources/Shaders/vertex_shader.vert", "Resources/Shaders/fragment_shader.frag");
    outlineShaderProgram = shaderLoader.CreateProgram("Resources/Shaders/outline_vertex_shader.vert", "Resources/Shaders/outline_fragment_shader.frag");
}

void StencilTestScene::render() {
    // Clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Render the skybox first
    glDepthMask(GL_FALSE);
    glm::mat4 view = glm::mat4(glm::mat3(camera.getViewMatrix()));
    skybox.render(camera.getProjectionMatrix() * view);
    glDepthMask(GL_TRUE);

    // Set up stencil buffer
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);

    glUseProgram(shaderProgram);

    // Model matrix with rotation
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    float rotationAngle = glm::radians(glfwGetTime() * 20.0f);
    modelMatrix = glm::rotate(modelMatrix, rotationAngle, glm::vec3(0.0f, 1.0f, 0.0f));

    glm::mat4 viewMatrix = camera.getViewMatrix();
    glm::mat4 projectionMatrix = camera.getProjectionMatrix();

    // Set uniforms for the regular object
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &modelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

    // Render the model normally
    renderer.render(shaderProgram, modelMatrix);

    // Render the outline
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);
    glDisable(GL_DEPTH_TEST);

    glUseProgram(outlineShaderProgram);

    float outlineScale = 1.05f; // Adjust this scale to control the outline thickness
    glm::mat4 outlineModelMatrix = glm::scale(modelMatrix, glm::vec3(outlineScale));

    glUniformMatrix4fv(glGetUniformLocation(outlineShaderProgram, "model"), 1, GL_FALSE, &outlineModelMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(outlineShaderProgram, "view"), 1, GL_FALSE, &viewMatrix[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(outlineShaderProgram, "projection"), 1, GL_FALSE, &projectionMatrix[0][0]);

    renderer.render(outlineShaderProgram, outlineModelMatrix);

    // Reset stencil and depth settings
    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 0, 0xFF);
    glEnable(GL_DEPTH_TEST);
}


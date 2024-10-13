#include "InstancedRenderer.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include <iostream>
#include "ModelLoader.h"

InstancedRenderer::InstancedRenderer(const std::string& modelPath, const std::string& texturePath, int instanceCount)
    : texture(texturePath), instanceCount(instanceCount) {
    loadModelData(modelPath);
}

InstancedRenderer::~InstancedRenderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &texCoordVBO);
    glDeleteBuffers(1, &normalVBO);
    glDeleteBuffers(1, &instanceVBO);
}

void InstancedRenderer::loadModelData(const std::string& modelPath) {
    ModelLoader modelLoader(modelPath);
    modelLoader.loadModel();

    positions = modelLoader.getPositions();
    texCoords = modelLoader.getTexCoords();
    normals = modelLoader.getNormals();
}

void InstancedRenderer::initialize() {
    std::vector<glm::mat4> instanceMatrices(instanceCount);

    int gridSize = static_cast<int>(sqrt(instanceCount)); // Calculate grid size
    float spacing = 300.0f; // Adjust spacing between objects

    for (int i = 0; i < instanceCount; ++i) {
        int row = i / gridSize;
        int col = i % gridSize;
        float x = col * spacing;
        float y = 0.0f;
        float z = row * spacing;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, y, z));
        model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.3f, 0.5f)); // No rotation
        model = glm::scale(model, glm::vec3(0.3f)); // Scale objects
        instanceMatrices[i] = model;
    }

    // Set up VAO, VBOs
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &texCoordVBO);
    glGenBuffers(1, &normalVBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);

    // Position Buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture Coordinate Buffer
    glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(1);

    // Normal Buffer
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(2);

    // Instance Matrix Buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceCount * sizeof(glm::mat4), instanceMatrices.data(), GL_STATIC_DRAW);

    for (int i = 0; i < 4; i++) {
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
        glEnableVertexAttribArray(3 + i);
        glVertexAttribDivisor(3 + i, 1);
    }

    glBindVertexArray(0);
}

void InstancedRenderer::render(GLuint shaderProgram, const glm::mat4& viewProjectionMatrix) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewProjectionMatrix"), 1, GL_FALSE, &viewProjectionMatrix[0][0]);

    texture.bind();
    glBindVertexArray(VAO);
    glDrawArraysInstanced(GL_TRIANGLES, 0, positions.size(), instanceCount);
    glBindVertexArray(0);
    texture.unbind();
}

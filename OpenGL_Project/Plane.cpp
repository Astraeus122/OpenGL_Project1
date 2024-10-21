#include "Plane.h"

Plane::Plane() : ModelLoader("") {}

void Plane::loadPlane() {
    // Define plane vertices (positions, normals, texcoords)
    float planeVertices[] = {
        // positions            // normals         // texcoords
        500.0f, 0.0f,  500.0f, 0.0f, 1.0f, 0.0f,  50.0f, 0.0f,
       -500.0f, 0.0f,  500.0f, 0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
       -500.0f, 0.0f, -500.0f, 0.0f, 1.0f, 0.0f,  0.0f, 50.0f,

        500.0f, 0.0f,  500.0f, 0.0f, 1.0f, 0.0f,  50.0f, 0.0f,
       -500.0f, 0.0f, -500.0f, 0.0f, 1.0f, 0.0f,  0.0f, 50.0f,
        500.0f, 0.0f, -500.0f, 0.0f, 1.0f, 0.0f,  50.0f, 50.0f
    };

    // Set up buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

    // Positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // TexCoords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Plane::render(GLuint shaderProgram, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix) {
    glBindVertexArray(VAO);

    // Optionally pass matrices to the shader if needed
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    glPatchParameteri(GL_PATCH_VERTICES, 3);  // Ensure triangle patches for tessellation
    glDrawArrays(GL_PATCHES, 0, 6);  // Draw 6 vertices forming 2 triangles (1 patch)

    glBindVertexArray(0);
}


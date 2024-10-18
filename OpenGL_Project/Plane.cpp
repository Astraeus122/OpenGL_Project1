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

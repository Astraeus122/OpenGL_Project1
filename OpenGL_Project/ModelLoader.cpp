#define TINYOBJLOADER_IMPLEMENTATION
#include "ModelLoader.h"
#include "dependencies/tiny_obj_loader.h"
#include <iostream>

ModelLoader::ModelLoader(const std::string& modelPath)
    : modelPath(modelPath), VAO(0), VBO(0), EBO(0), modelMatrix(glm::mat4(1.0f)) {}

ModelLoader::~ModelLoader() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void ModelLoader::loadModel() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str());
    if (!ret) {
        std::cerr << "Failed to load model: " << warn << err << std::endl;
        return;
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            positions.emplace_back(glm::vec3(
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            ));

            texCoords.emplace_back(glm::vec2(
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            ));

            normals.emplace_back(glm::vec3(
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            ));

            indices.emplace_back(static_cast<unsigned int>(indices.size()));
        }
    }

    setupMesh();
}

void ModelLoader::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO for vertex data (positions, normals, texcoords)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
        positions.size() * sizeof(glm::vec3) +
        normals.size() * sizeof(glm::vec3) +
        texCoords.size() * sizeof(glm::vec2),
        nullptr, GL_STATIC_DRAW);

    // Load positions
    glBufferSubData(GL_ARRAY_BUFFER, 0, positions.size() * sizeof(glm::vec3), positions.data());

    // Load normals
    glBufferSubData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), normals.size() * sizeof(glm::vec3), normals.data());

    // Load texture coordinates
    glBufferSubData(GL_ARRAY_BUFFER,
        positions.size() * sizeof(glm::vec3) + normals.size() * sizeof(glm::vec3),
        texCoords.size() * sizeof(glm::vec2), texCoords.data());

    // Setup indices for EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Define vertex attributes
    // Positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // Normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
        (void*)(positions.size() * sizeof(glm::vec3)));
    glEnableVertexAttribArray(1);

    // Texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2),
        (void*)(positions.size() * sizeof(glm::vec3) + normals.size() * sizeof(glm::vec3)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void ModelLoader::render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& projection) {
    glUseProgram(shaderProgram);

    // Set transformation matrices
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// Transformation Methods
void ModelLoader::rotate(float radians, const glm::vec3& axis) {
    modelMatrix = glm::rotate(modelMatrix, radians, axis);
}

void ModelLoader::translate(const glm::vec3& offset) {
    std::cout << "Translating model by: " << offset.x << ", " << offset.y << ", " << offset.z << std::endl;
    modelMatrix = glm::translate(modelMatrix, offset);  // Apply translation
}

void ModelLoader::scale(const glm::vec3& scaleFactor) {
    modelMatrix = glm::scale(modelMatrix, scaleFactor) * modelMatrix;
}

void ModelLoader::resetTransformation() {
    modelMatrix = glm::mat4(1.0f);
}

// Getters
const std::vector<glm::vec3>& ModelLoader::getPositions() const {
    return positions;
}

const std::vector<glm::vec2>& ModelLoader::getTexCoords() const {
    return texCoords;
}

const std::vector<glm::vec3>& ModelLoader::getNormals() const {
    return normals;
}

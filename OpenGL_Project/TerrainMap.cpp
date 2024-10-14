// TerrainMap.cpp

#include "TerrainMap.h"
#include <fstream>
#include <iostream>
#include "Dependencies/stb_image.h" 
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "ShaderLoader.h" 
#include "Dependencies/glm/gtc/type_ptr.hpp"

// Constructor
TerrainMap::TerrainMap(const std::string& heightmapFile, int width, int height, float maxHeight)
    : heightmapFile(heightmapFile), width(width), height(height), maxHeight(maxHeight), vao(0), vbo(0), ebo(0),
    grassTexture(0), dirtTexture(0), rockTexture(0), snowTexture(0), shaderProgram(0) {}

// Destructor
TerrainMap::~TerrainMap() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteTextures(1, &grassTexture);
    glDeleteTextures(1, &dirtTexture);
    glDeleteTextures(1, &rockTexture);
    glDeleteTextures(1, &snowTexture);
    if (shaderProgram) glDeleteProgram(shaderProgram);
}

// Initialize the terrain map
void TerrainMap::initialize() {
    loadHeightmapData();
    createTerrainMesh();
    computeNormals();
    loadTextures();
}

// Load heightmap data from the file
void TerrainMap::loadHeightmapData() {
    std::ifstream file(heightmapFile, std::ios::binary);
    heightmap.resize(width * height);
    if (file.is_open()) {
        file.read(reinterpret_cast<char*>(&heightmap[0]), width * height);
        file.close();
    }
    else {
        std::cerr << "Failed to open heightmap file: " << heightmapFile << std::endl;
    }
}

// Create the terrain mesh
void TerrainMap::createTerrainMesh() {
    vertices.clear();
    normals.clear();
    indices.clear();

    for (int z = 0; z < height; ++z) {
        for (int x = 0; x < width; ++x) {
            float y = static_cast<float>(heightmap[z * width + x]) / 255.0f * maxHeight;
            vertices.push_back(glm::vec3(x, y, z));
            normals.push_back(glm::vec3(0, 1, 0)); // Placeholder normal
        }
    }

    for (int z = 0; z < height - 1; ++z) {
        for (int x = 0; x < width - 1; ++x) {
            indices.push_back(z * width + x);
            indices.push_back((z + 1) * width + x);
            indices.push_back(z * width + (x + 1));

            indices.push_back(z * width + (x + 1));
            indices.push_back((z + 1) * width + x);
            indices.push_back((z + 1) * width + (x + 1));
        }
    }

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

// Compute normals for the terrain
void TerrainMap::computeNormals() {
    std::fill(normals.begin(), normals.end(), glm::vec3(0.0f, 0.0f, 0.0f));

    for (size_t i = 0; i < indices.size(); i += 3) {
        glm::vec3 v0 = vertices[indices[i]];
        glm::vec3 v1 = vertices[indices[i + 1]];
        glm::vec3 v2 = vertices[indices[i + 2]];
        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));

        normals[indices[i]] += normal;
        normals[indices[i + 1]] += normal;
        normals[indices[i + 2]] += normal;
    }

    for (auto& normal : normals) {
        normal = glm::normalize(normal);
    }
}

// Load textures for the terrain
void TerrainMap::loadTextures() {
    grassTexture = loadTexture("Resources/Textures/PolygonScifiWorlds_Texture_01_B.png");
    dirtTexture = loadTexture("Resources/Textures/PolygonAncientWorlds_Statue_01.png");
    rockTexture = loadTexture("Resources/Textures/PolygonAncientWorlds_Texture_01_A.png");
    snowTexture = loadTexture("Resources/Textures/PolygonAncientWorlds_Texture_01_B.png");
}

// Helper function to load a texture
GLuint TerrainMap::loadTexture(const std::string& filePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filePath.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 3) ? GL_RGB : GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cerr << "Texture failed to load at path: " << filePath << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// Render the terrain for the shadow pass
void TerrainMap::renderShadow(GLuint shadowShaderProgram) {
    glUseProgram(shadowShaderProgram);

    // Pass the model matrix to the shadow shader
    glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Render the terrain mesh
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void TerrainMap::renderNormal(GLuint lightingShaderProgram) {
    glUseProgram(lightingShaderProgram);

    // Set the 'isTerrain' uniform to true
    glUniform1i(glGetUniformLocation(lightingShaderProgram, "isTerrain"), 1);

    // Pass the model matrix to the lighting shader
    glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Render the terrain mesh
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void TerrainMap::resetTransformation() {
    modelMatrix = glm::mat4(1.0f);
}

void TerrainMap::translate(const glm::vec3& offset) {
    modelMatrix = glm::translate(modelMatrix, offset);
}

void TerrainMap::scale(const glm::vec3& scaleFactor) {
    modelMatrix = glm::scale(modelMatrix, scaleFactor);
}

#ifndef TERRAINMAP_H
#define TERRAINMAP_H

#include <string>
#include <vector>
#include <glew.h>
#include "Dependencies/glm/glm.hpp"

class TerrainMap {
public:
    TerrainMap(const std::string& heightmapFile, int width, int height, float maxHeight);
    ~TerrainMap();

    void initialize();
    void renderShadow(GLuint shadowShaderProgram); // For shadow pass
    void renderNormal(GLuint lightingShaderProgram); // For normal rendering

    // Transformation methods
    void resetTransformation();
    void translate(const glm::vec3& offset);
    void scale(const glm::vec3& scaleFactor);

    glm::mat4 getModelMatrix() const { return modelMatrix; } // Ensure this returns a glm::mat4
    GLuint getHeightmapTextureID() const { return grassTexture; }  // Replace with the actual texture ID variable


private:
    std::string heightmapFile;
    int width, height;
    float maxHeight;
    std::vector<unsigned char> heightmap;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<GLuint> indices;

    GLuint vao, vbo, ebo;
    GLuint grassTexture, dirtTexture, rockTexture, snowTexture;
    GLuint shaderProgram;

    glm::mat4 modelMatrix; // Transformation matrix

    void loadHeightmapData();
    void createTerrainMesh();
    void computeNormals();
    void loadTextures();
    GLuint loadTexture(const std::string& filePath);
};

#endif // TERRAINMAP_H

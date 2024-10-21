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

    glm::mat4 getModelMatrix() const { return modelMatrix; } // Getter for model matrix

    // New methods to fix the current errors
    GLuint getVAO() const { return vao; }
    size_t getIndexCount() const { return indices.size(); }
    GLuint loadTexture(const std::string& filePath);

private:
    std::string heightmapFile;
    int width, height;
    float maxHeight;
    std::vector<unsigned char> heightmap;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<GLuint> indices;  // Ensure this is defined to track index data

    GLuint vao, vbo, ebo;
    GLuint grassTexture, dirtTexture, rockTexture, snowTexture;
    GLuint shaderProgram;

    glm::mat4 modelMatrix; // Transformation matrix

    void loadHeightmapData();
    void createTerrainMesh();
    void computeNormals();
    void loadTextures();
    
};

#endif // TERRAINMAP_H

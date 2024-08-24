// TerrainMap.h

#ifndef TERRAIN_MAP_H
#define TERRAIN_MAP_H

#include <vector>
#include <string>
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/GLEW/glew.h"

class TerrainMap {
public:
    TerrainMap(const std::string& heightmapFile, int width, int height, float maxHeight);
    ~TerrainMap();

    void initialize();
    void render();

private:
    std::string heightmapFile;
    int width, height;
    float maxHeight;
    GLuint vao, vbo, ebo;
    GLuint grassTexture, dirtTexture, rockTexture, snowTexture; // Texture handles
    GLuint shaderProgram; // Shader program for terrain
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<GLuint> indices;
    std::vector<unsigned char> heightmap;

    void loadHeightmapData();
    void createTerrainMesh();
    void computeNormals();
    void loadTextures();
    GLuint loadTexture(const std::string& filePath);
};

#endif // TERRAIN_MAP_H

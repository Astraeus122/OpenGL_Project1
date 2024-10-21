#ifndef LODSCENE_H
#define LODSCENE_H

#include "Dependencies/GLEW/glew.h"
#include "Dependencies/glm/glm.hpp"
#include "ShaderLoader.h"
#include "Camera.h"
#include "Texture.h"
#include <string>
#include <vector>

class LODScene
{
public:
    // Constructor and Destructor
    LODScene(ShaderLoader& shaderLoader, Camera& camera);
    ~LODScene();

    // Initialize the scene (load shaders, set up geometry, load textures)
    void initialize();

    // Render the scene
    void render();

private:
    // References to ShaderLoader and Camera
    ShaderLoader& shaderLoader;
    Camera& camera;

    // Shader Programs
    GLuint triangleProgram;
    GLuint quadProgram;
    GLuint terrainProgram;

    // Vertex Array Objects and Vertex Buffer Objects
    GLuint triangleVAO, triangleVBO;
    GLuint quadVAO, quadVBO, quadEBO;
    GLuint terrainVAO, terrainVBO, terrainEBO;

    // Textures
    GLuint triangleTexture;
    GLuint quadTexture;
    GLuint terrainHeightmap;
    GLuint terrainTexture;

    // Terrain Parameters
    int terrainResolution; // e.g., 32 for a 32x32 grid
    std::vector<float> terrainVertices;
    std::vector<unsigned int> terrainIndices;

    // Helper Methods
    void setupTriangle();
    void setupQuad();
    void setupTerrain();
    void calculateTerrainGeometry();
    void loadTextures();
};

#endif 
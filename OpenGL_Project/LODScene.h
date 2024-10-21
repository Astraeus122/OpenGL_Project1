#ifndef LODSCENE_H
#define LODSCENE_H

#include "ShaderLoader.h"
#include "Camera.h"
#include "TerrainMap.h"  // Ensure this is the correct path to TerrainMap

class LODScene {
public:
    LODScene(ShaderLoader& shaderLoader, Camera& camera);

    void initialize();
    void update(float deltaTime);
    void render();

private:
    GLuint shaderProgram;
    GLuint vertexShader, tessControlShader, tessEvalShader, fragmentShader;

    Camera& camera;
    TerrainMap terrain;  // Terrain object for handling the terrain

    glm::mat4 modelMatrix;

    bool wireframeMode;

    Texture* heightmapTexture;
    Texture* terrainTexture;



    // Window handle for GLFW input
    GLFWwindow* window;
};

#endif

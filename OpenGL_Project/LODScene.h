#ifndef LODSCENE_H
#define LODSCENE_H

#include "ShaderLoader.h"
#include "Camera.h"
#include "TerrainMap.h"  // Corrected class name

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
    TerrainMap terrain;  // Corrected class name

    glm::mat4 modelMatrix;

    bool wireframeMode;

    // Window handle for GLFW input
    GLFWwindow* window;
};

#endif

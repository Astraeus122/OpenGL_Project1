#ifndef PERLIN_NOISE_SCENE_H
#define PERLIN_NOISE_SCENE_H

#include "ShaderLoader.h"
#include "Camera.h"
#include <glew.h>
#include <glfw3.h>
#include "Dependencies/glm/glm.hpp"
#include <vector>

class PerlinNoiseScene {
public:
    PerlinNoiseScene(ShaderLoader& shaderLoader, Camera& camera);
    void initialize();
    void render();
    void update(float deltaTime);

private:
    ShaderLoader& m_shaderLoader;
    Camera& m_camera;
    GLuint m_terrainShaderProgram;
    GLuint m_2dNoiseShaderProgram;
    GLuint m_terrainVAO, m_terrainVBO, m_terrainEBO;
    GLuint m_2dQuadVAO, m_2dQuadVBO;
    GLuint m_terrainTexture;
    GLuint m_2dNoiseTexture;
    std::vector<int> p;
    float m_time;

    // Perlin noise functions
    void initializeNoise();
    double fade(double t);
    double lerp(double t, double a, double b);
    double grad(int hash, double x, double y, double z);
    double noise(double x, double y, double z);

    void generateAndSavePerlinNoiseImage(const std::string& jpgFilePath, const std::string& rawFilePath);
    void generateTerrainMesh();
    void loadTerrainTexture();
    void setup2DQuad();
    void generate2DNoiseTexture();
};

#endif // PERLIN_NOISE_SCENE_H
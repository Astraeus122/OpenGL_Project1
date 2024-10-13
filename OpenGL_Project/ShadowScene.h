#ifndef SHADOWSCENE_H
#define SHADOWSCENE_H

#include "ShaderLoader.h"
#include "Camera.h"
#include "Skybox.h"
#include "InstancedRenderer.h"
#include "LightManager.h"
#include "ShadowMap.h"
#include "TerrainMap.h"
#include "ModelLoader.h"
#include <glew.h>
#include <vector>
#include "Dependencies/glm/glm.hpp"

class ShadowScene
{
public:
    ShadowScene(ShaderLoader& shaderLoader, Camera& camera, Skybox& skybox, InstancedRenderer& renderer, LightManager& lightManager);
    void initialize();
    void render();
    void renderShadowPass(int lightIndex);
    void moveModel(const glm::vec3& newPosition);

private:
    ShaderLoader& shaderLoader;
    Camera& camera;
    Skybox& skybox;
    InstancedRenderer& renderer;
    LightManager& lightManager;

    std::vector<ShadowMap> shadowMaps; // Shadow maps for directional lights
    std::vector<glm::vec3> lightDirections; // Directions for directional lights
    std::vector<glm::mat4> lightSpaceMatrices; // Light space matrices

    TerrainMap terrain;
    std::vector<ModelLoader> models; // List of loaded models

    // Position vectors for each model
    std::vector<glm::vec3> modelPositions;

    GLuint shadowShaderProgram;
    GLuint lightingShaderProgram;

    // Movable model index
    int movableModelIndex;

    void renderSceneWithShadows();
    void setupLights();
};

#endif

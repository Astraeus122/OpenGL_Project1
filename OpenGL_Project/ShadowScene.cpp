#include "ShadowScene.h"
#include <glew.h>
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"

ShadowScene::ShadowScene(ShaderLoader& shaderLoader, Camera& camera, Skybox& skybox, InstancedRenderer& renderer, LightManager& lightManager)
    : shaderLoader(shaderLoader), camera(camera), skybox(skybox), renderer(renderer), lightManager(lightManager),
    terrain("Resources/Heightmap0.raw", 128, 128, 20.0f),
    movableModelIndex(-1) // Initialize with invalid index
{
    shadowShaderProgram = shaderLoader.CreateProgram("Resources/Shaders/shadow_vertex_shader.txt", "Resources/Shaders/shadow_fragment_shader.txt");
    lightingShaderProgram = shaderLoader.CreateProgram("Resources/Shaders/lighting_vertex_shader.txt", "Resources/Shaders/lighting_fragment_shader.txt");

    // Initialize models with distinct positions
    models.emplace_back("Resources/Models/AncientEmpire/SM_Wep_Axe_02.obj");
    models.emplace_back("Resources/Models/SciFiWorlds/SM_Wep_Sword_02.obj");
    models.emplace_back("Resources/Models/SciFiSpace/SM_Ship_Fighter_02.obj"); 

    // Initialize model positions
    modelPositions.emplace_back(glm::vec3(-10.0f, 0.0f, 0.0f)); // Position for first model
    modelPositions.emplace_back(glm::vec3(10.0f, 0.0f, 0.0f));  // Position for second model
    modelPositions.emplace_back(glm::vec3(0.0f, 0.0f, 20.0f));  // Position for movable model

    // Designate the third model as movable
    movableModelIndex = 2;

    // Initialize shadow maps for two directional lights
    shadowMaps.resize(2);
    for (auto& shadowMap : shadowMaps)
    {
        shadowMap.init(2048, 2048);
    }

    // Set light directions
    lightDirections.emplace_back(glm::vec3(-0.5f, -1.0f, -0.5f)); // Light 1 direction
    lightDirections.emplace_back(glm::vec3(0.5f, -1.0f, 0.5f));   // Light 2 direction
}

void ShadowScene::initialize() {
    // Initialize terrain
    terrain.initialize();

    // Initialize models
    for (auto& model : models) {
        model.loadModel();
    }

    // Initialize renderer if needed
    renderer.initialize();
}

void ShadowScene::setupLights() {
    for (size_t i = 0; i < shadowMaps.size(); ++i) {
        shadowMaps[i].setLightPosition(lightDirections[i]);
        lightSpaceMatrices.push_back(shadowMaps[i].getLightSpaceMatrix());
    }
}

void ShadowScene::render() {
    // Setup lights and update light space matrices
    setupLights();

    // Render shadow pass for each light source
    for (size_t i = 0; i < shadowMaps.size(); ++i) {
        renderShadowPass(i);
    }

    // Render the scene with shadows
    renderSceneWithShadows();
}

void ShadowScene::renderShadowPass(int lightIndex) {
    shadowMaps[lightIndex].bind();
    glUseProgram(shadowShaderProgram);

    shadowMaps[lightIndex].setLightPosition(lightDirections[lightIndex]);
    glm::mat4 lightSpaceMatrix = shadowMaps[lightIndex].getLightSpaceMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    // Render terrain and models to shadow map
    terrain.render();

    for (size_t i = 0; i < models.size(); ++i) {
        // Set model matrix with position and scaling
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        // Apply scaling first
        float scaleFactor = (i == movableModelIndex) ? 0.0005f : 0.0001f;
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleFactor));

        // Apply translation
        modelMatrix = glm::translate(modelMatrix, modelPositions[i]);

        glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

        models[i].render(shadowShaderProgram, glm::mat4(1.0f), glm::mat4(1.0f));
    }

    shadowMaps[lightIndex].unbind();
}

void ShadowScene::renderSceneWithShadows() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(lightingShaderProgram);

    for (size_t i = 0; i < shadowMaps.size(); ++i) {
        std::string lightSpaceUniform = "lightSpaceMatrix" + std::to_string(i + 1);
        glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, lightSpaceUniform.c_str()), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrices[i]));

        std::string shadowMapUniform = "shadowMap" + std::to_string(i + 1);
        glUniform1i(glGetUniformLocation(lightingShaderProgram, shadowMapUniform.c_str()), i);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, shadowMaps[i].getDepthMap());

        std::string lightDirUniform = "lightDir" + std::to_string(i + 1);
        glUniform3fv(glGetUniformLocation(lightingShaderProgram, lightDirUniform.c_str()), 1, &lightDirections[i][0]);
    }

    glUniform3fv(glGetUniformLocation(lightingShaderProgram, "viewPos"), 1, glm::value_ptr(camera.getPosition()));

    // Render terrain with transformations
    glm::mat4 terrainModel = glm::mat4(1.0f);
    terrainModel = glm::translate(terrainModel, glm::vec3(0.0f, -0.3f, 0.0f));
    terrainModel = glm::scale(terrainModel, glm::vec3(20.0f, 50.0f, 20.0f));
    glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(terrainModel));
    terrain.render();

    // Render models with their respective positions and scaling
    for (size_t i = 0; i < models.size(); ++i) {
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        // Apply scaling first
        float scaleFactor = (i == movableModelIndex) ? 0.0005f : 0.0001f; // Scale down the movable model more
        modelMatrix = glm::scale(modelMatrix, glm::vec3(scaleFactor));

        // Apply translation
        modelMatrix = glm::translate(modelMatrix, modelPositions[i]);

        glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        models[i].render(lightingShaderProgram, camera.GetViewMatrix(), camera.GetProjectionMatrix());
    }

    // Render instances if needed
    renderer.render(lightingShaderProgram, camera.GetViewMatrix() * camera.GetProjectionMatrix());
}

void ShadowScene::moveModel(const glm::vec3& newPosition)
{
    if (movableModelIndex >= 0 && movableModelIndex < modelPositions.size()) {
        modelPositions[movableModelIndex] = newPosition;
    }
}

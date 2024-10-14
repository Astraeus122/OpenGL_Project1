#include "ShadowScene.h"

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

void ShadowScene::initialize()
{
    // Initialize terrain
    terrain.initialize();

    // Apply transformations to the terrain
    terrain.resetTransformation();
    terrain.translate(glm::vec3(-150.0f, -30.0f, -50.0f));
    terrain.scale(glm::vec3(20.0f, 5.0f, 20.0f));    // Reduced Y scaling

    // Initialize models
    for (size_t i = 0; i < models.size(); ++i) {
        models[i].loadModel();
        if (i == static_cast<size_t>(movableModelIndex)) {
            // Initialize movable model's position
            models[i].translate(glm::vec3(0.0f, 0.0f, 20.0f));

            // Scale down the movable model
            models[i].scale(glm::vec3(0.05f)); // Scale to 5% of its original size
        }
        else {
            // Initialize other models' positions
            if (i == 0)
                models[i].translate(glm::vec3(-10.0f, 0.0f, 0.0f));
            else if (i == 1)
                models[i].translate(glm::vec3(10.0f, 0.0f, 0.0f));

            // Set the scale for other models
            models[i].scale(glm::vec3(1.0f));
        }
    }

    // Initialize renderer if needed
    renderer.initialize();
}

void ShadowScene::renderShadowPass(int lightIndex) {
    shadowMaps[lightIndex].bind();
    glUseProgram(shadowShaderProgram);

    // Set light space matrix
    shadowMaps[lightIndex].setLightPosition(lightDirections[lightIndex]);
    glm::mat4 lightSpaceMatrix = shadowMaps[lightIndex].getLightSpaceMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    // Render terrain with shadow shader
    terrain.renderShadow(shadowShaderProgram);

    // Render all models with shadow shader
    for (size_t i = 0; i < models.size(); ++i) {
        glm::mat4 modelMatrix = models[i].getModelMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shadowShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        models[i].render(shadowShaderProgram, camera.GetViewMatrix(), camera.GetProjectionMatrix());
    }

    shadowMaps[lightIndex].unbind();
}

void ShadowScene::renderSceneWithShadows() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(lightingShaderProgram);

    // Set light space matrices and shadow maps
    for (size_t i = 0; i < shadowMaps.size(); ++i) {
        // Set lightSpaceMatrixi
        std::string lightSpaceUniform = "lightSpaceMatrix" + std::to_string(i + 1);
        glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, lightSpaceUniform.c_str()), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrices[i]));

        // Set shadowMapi to texture unit i
        std::string shadowMapUniform = "shadowMap" + std::to_string(i + 1);
        glUniform1i(glGetUniformLocation(lightingShaderProgram, shadowMapUniform.c_str()), static_cast<int>(i));
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_2D, shadowMaps[i].getDepthMap());

        // Set lightDiri
        std::string lightDirUniform = "lightDir" + std::to_string(i + 1);
        glUniform3fv(glGetUniformLocation(lightingShaderProgram, lightDirUniform.c_str()), 1, &lightDirections[i][0]);
    }

    glUniform3fv(glGetUniformLocation(lightingShaderProgram, "viewPos"), 1, glm::value_ptr(camera.getPosition()));

    // Render terrain first
    glUniform1i(glGetUniformLocation(lightingShaderProgram, "isTerrain"), 1);
    glUniform1f(glGetUniformLocation(lightingShaderProgram, "maxHeight"), 20.0f); // Set to terrain's max height
    terrain.renderNormal(lightingShaderProgram);

    // Render models after terrain
    glUniform1i(glGetUniformLocation(lightingShaderProgram, "isTerrain"), 0);
    glUniform1f(glGetUniformLocation(lightingShaderProgram, "maxHeight"), 1.0f); // Default for models
    for (size_t i = 0; i < models.size(); ++i) {
        glm::mat4 modelMatrix = models[i].getModelMatrix();

        glUniformMatrix4fv(glGetUniformLocation(lightingShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        models[i].render(lightingShaderProgram, camera.GetViewMatrix(), camera.GetProjectionMatrix());
    }

    // Render instances 
    renderer.render(lightingShaderProgram, camera.GetViewMatrix() * camera.GetProjectionMatrix());
}

void ShadowScene::setupLights() {
    lightSpaceMatrices.clear(); // Clear previous matrices
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
        renderShadowPass(static_cast<int>(i));
    }

    // Render the scene with shadows
    renderSceneWithShadows();
}

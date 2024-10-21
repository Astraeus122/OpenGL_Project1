#include "LODScene.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"
#include "Texture.h"  // Add the Texture class
#include <iostream>

LODScene::LODScene(ShaderLoader& shaderLoader, Camera& camera)
    : camera(camera), terrain("Resources/Heightmap0.raw", 128, 128, 20.0f), wireframeMode(false)
{
    // Initialize the textures
    heightmapTexture = new Texture("Resources/Heightmap0.raw");
    terrainTexture = new Texture("Resources/Textures/PolygonScifiWorlds_Texture_01_B.png");
    // Load and compile shaders
    vertexShader = shaderLoader.CreateShader(GL_VERTEX_SHADER, "vertex_shader.glsl");
    tessControlShader = shaderLoader.CreateShader(GL_TESS_CONTROL_SHADER, "tess_control_shader.glsl");
    tessEvalShader = shaderLoader.CreateShader(GL_TESS_EVALUATION_SHADER, "tess_evaluation_shader.glsl");
    fragmentShader = shaderLoader.CreateShader(GL_FRAGMENT_SHADER, "lod_fragment_shader.glsl");

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, tessControlShader);
    glAttachShader(shaderProgram, tessEvalShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Error linking shader program: " << infoLog << std::endl;
    }
}

void LODScene::initialize() {
    // Initialize terrain
    terrain.initialize();

    // Load textures
    Texture heightmapTex("Resources/Heightmap0.raw");
    Texture terrainTex("Resources/Textures/PolygonScifiWorlds_Texture_01_B.png");

    // Store texture IDs for later use
    heightmapTexture = heightmapTex.getID();
    terrainTexture = terrainTex.getID();

    // Apply transformations to the terrain
    terrain.resetTransformation();
    terrain.translate(glm::vec3(0.0f, -50.0f, 0.0f));
    terrain.scale(glm::vec3(50.0f, 1.0f, 50.0f));
}

void LODScene::render() {
    glUseProgram(shaderProgram);

    // Pass model, view, and projection matrices to the shaders
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(terrain.getModelMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));

    // Pass camera position for LOD calculations
    glUniform3fv(glGetUniformLocation(shaderProgram, "cameraPos"), 1, glm::value_ptr(camera.getPosition()));

    heightmapTexture->bind();
    glUniform1i(glGetUniformLocation(shaderProgram, "heightmap"), 0);

    terrainTexture->bind();
    glUniform1i(glGetUniformLocation(shaderProgram, "terrainTexture"), 1);


    // Set number of vertices per patch
    glPatchParameteri(GL_PATCH_VERTICES, 3);

    // Bind VAO and draw terrain
    glBindVertexArray(terrain.getVAO());
    glDrawElements(GL_PATCHES, terrain.getIndexCount(), GL_UNSIGNED_INT, 0);

    // Unbind VAO and textures
    glBindVertexArray(0);
    heightmapTexture.unbind();
    terrainTexture.unbind();

    glUseProgram(0);
}

LODScene::~LODScene() {
    delete heightmapTexture;
    delete terrainTexture;
}

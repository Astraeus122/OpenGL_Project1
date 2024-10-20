#include "LODScene.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"
#include <iostream>

LODScene::LODScene(ShaderLoader& shaderLoader, Camera& camera)
    : camera(camera), terrain() {

    // Load and compile shaders
    vertexShader = shaderLoader.CreateShader(GL_VERTEX_SHADER, "vertex_shader.glsl");
    tessControlShader = shaderLoader.CreateShader(GL_TESS_CONTROL_SHADER, "tess_control_shader.glsl");
    tessEvalShader = shaderLoader.CreateShader(GL_TESS_EVALUATION_SHADER, "tess_evaluation_shader.glsl");
    fragmentShader = shaderLoader.CreateShader(GL_FRAGMENT_SHADER, "fragment_shader.glsl");

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
    // Set up terrain model matrix
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -10.0f, 0.0f)); // Adjust position
    modelMatrix = glm::scale(modelMatrix, glm::vec3(50.0f, 1.0f, 50.0f)); // Scale terrain

    terrain.loadPlane();  // Load plane as the terrain
}

void LODScene::update(float deltaTime) {
    // Add updates (for LOD or camera movement)
}

void LODScene::render() {
    glUseProgram(shaderProgram);

    // Pass model, view, and projection matrices to shaders
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));

    // Pass camera position to tessellation control shader for LOD
    glUniform3fv(glGetUniformLocation(shaderProgram, "cameraPos"), 1, glm::value_ptr(camera.getPosition()));

    // Set the number of vertices per patch (for tessellation)
    glPatchParameteri(GL_PATCH_VERTICES, 3); // Triangle patches for tessellation

    // Render the terrain (plane)
    terrain.render(shaderProgram, camera.GetViewMatrix(), camera.GetProjectionMatrix());

    glUseProgram(0);
}

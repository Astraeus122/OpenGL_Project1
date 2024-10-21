#include "LODScene.h"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"
#include <iostream>

LODScene::LODScene(ShaderLoader& shaderLoader, Camera& camera)
    : camera(camera), terrain("Resources/Heightmap0.raw", 128, 128, 20.0f), wireframeMode(false)
{
    // After each shader creation, add error checking like this:
    vertexShader = shaderLoader.CreateShader(GL_VERTEX_SHADER, "vertex_shader.glsl");
    if (!vertexShader) {
        std::cerr << "Vertex Shader compilation failed!" << std::endl;
    }

   tessControlShader = shaderLoader.CreateShader(GL_TESS_CONTROL_SHADER, "tess_control_shader.glsl");
    if (!tessControlShader) {
        std::cerr << "Tessellation Control Shader compilation failed!" << std::endl;
    }

    tessEvalShader = shaderLoader.CreateShader(GL_TESS_EVALUATION_SHADER, "tess_evaluation_shader.glsl");
    if (!tessEvalShader) {
        std::cerr << "Tessellation Evaluation Shader compilation failed!" << std::endl;
    }

    fragmentShader = shaderLoader.CreateShader(GL_FRAGMENT_SHADER, "lod_fragment_shader.glsl");
    if (!fragmentShader) {
        std::cerr << "Fragment Shader compilation failed!" << std::endl;
    }


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

    // Apply transformations to the terrain
    terrain.resetTransformation();
    terrain.translate(glm::vec3(0.0f, -50.0f, 0.0f)); // Adjust position
    terrain.scale(glm::vec3(50.0f, 1.0f, 50.0f)); // Scale the terrain
}

void LODScene::update(float deltaTime) {
    // Toggle wireframe mode on 'T' keypress
    static bool tKeyPressed = false;
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        if (!tKeyPressed) {
            wireframeMode = !wireframeMode;  // Toggle wireframe mode
            if (wireframeMode) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // Enable wireframe
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Disable wireframe
            }
            tKeyPressed = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        tKeyPressed = false;
    }
}

void LODScene::render() {
    // Activate the shader program
    glUseProgram(shaderProgram);

    // Pass the model matrix to the shaders
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "modelMatrix"), 1, GL_FALSE, glm::value_ptr(terrain.getModelMatrix()));

    // Pass the view and projection matrices (camera)
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewMatrix"), 1, GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));

    // Pass the camera position for LOD and tessellation calculations
    glUniform3fv(glGetUniformLocation(shaderProgram, "cameraPos"), 1, glm::value_ptr(camera.getPosition()));

    // Bind the heightmap texture before rendering terrain
    glActiveTexture(GL_TEXTURE0);  // Set the active texture unit to 0
    glBindTexture(GL_TEXTURE_2D, terrain.getHeightmapTextureID());  // Bind the heightmap texture (replace with actual texture ID getter)
    glUniform1i(glGetUniformLocation(shaderProgram, "heightmap"), 0);  // Set the uniform for the heightmap in the shader

    // Render the terrain using tessellation
    terrain.renderNormal(shaderProgram);

    // Optionally print the camera position for debugging
    glm::vec3 camPos = camera.getPosition();
    std::cout << "Camera Position: " << camPos.x << ", " << camPos.y << ", " << camPos.z << std::endl;

    // Deactivate the shader program
    glUseProgram(0);
}

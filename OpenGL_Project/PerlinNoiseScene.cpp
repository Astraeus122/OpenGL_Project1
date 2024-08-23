#include "PerlinNoiseScene.h"
#include "Dependencies/stb_image.h"
#include <iostream>

PerlinNoiseScene::PerlinNoiseScene(ShaderLoader& shaderLoader, Camera& camera, Skybox& skybox)
    : shaderLoader(shaderLoader), camera(camera), skybox(skybox), perlinNoise((unsigned int)time(nullptr)) {
}

void PerlinNoiseScene::initialize() {
    perlinShader = shaderLoader.CreateProgram("perlin_vertex_shader.txt", "perlin_fragment_shader.txt");
    checkGLError("Shader compilation");
    generateNoiseTexture();
    setupQuad();
}

void PerlinNoiseScene::generateNoiseTexture() {
    std::string jpgFilePath = "perlin_noise.jpg";
    std::string rawFilePath = "perlin_noise.raw";
    perlinNoise.generateAndSavePerlinNoiseImage(jpgFilePath, rawFilePath);

    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(jpgFilePath.c_str(), &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        std::cout << "Texture loaded successfully. Size: " << width << "x" << height << ", Channels: " << nrChannels << std::endl;
    }
    else {
        std::cerr << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    checkGLError("Texture generation");
}

void PerlinNoiseScene::setupQuad() {
    float quadVertices[] = {
        // positions        // texture coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 0.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    checkGLError("Quad setup");
}

void PerlinNoiseScene::render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render skybox
    glDepthMask(GL_FALSE);
    glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.getViewMatrix()));
    skybox.render(camera.getProjectionMatrix() * skyboxView);
    glDepthMask(GL_TRUE);
    checkGLError("Skybox rendering");

    // Render Perlin noise quad
    glUseProgram(perlinShader);

    // Position the quad in front of the camera
    glm::vec3 quadPos = camera.position + camera.front * 5.0f;
    quadModelMatrix = glm::translate(glm::mat4(1.0f), quadPos);
    quadModelMatrix = glm::scale(quadModelMatrix, glm::vec3(5.0f));

    // Make the quad face the camera
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(camera.front, up));
    up = glm::cross(right, camera.front);
    glm::mat4 rotationMatrix = glm::mat4(
        glm::vec4(right, 0.0f),
        glm::vec4(up, 0.0f),
        glm::vec4(-camera.front, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    quadModelMatrix = quadModelMatrix * rotationMatrix;

    // Set uniforms
    GLint modelLoc = glGetUniformLocation(perlinShader, "model");
    GLint viewLoc = glGetUniformLocation(perlinShader, "view");
    GLint projLoc = glGetUniformLocation(perlinShader, "projection");
    GLint textureSamplerLoc = glGetUniformLocation(perlinShader, "textureSampler");

    if (modelLoc != -1) glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &quadModelMatrix[0][0]);
    if (viewLoc != -1) glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &camera.getViewMatrix()[0][0]);
    if (projLoc != -1) glUniformMatrix4fv(projLoc, 1, GL_FALSE, &camera.getProjectionMatrix()[0][0]);

    // Bind the noise texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    if (textureSamplerLoc != -1) glUniform1i(textureSamplerLoc, 0);

    // Render the quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkGLError("Quad rendering");

    // Print debug information
    std::cout << "Perlin shader program ID: " << perlinShader << std::endl;
    std::cout << "Noise texture ID: " << noiseTexture << std::endl;
    std::cout << "Model matrix location: " << modelLoc << std::endl;
    std::cout << "View matrix location: " << viewLoc << std::endl;
    std::cout << "Projection matrix location: " << projLoc << std::endl;
    std::cout << "Texture sampler location: " << textureSamplerLoc << std::endl;
}

void PerlinNoiseScene::checkGLError(const char* operation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after " << operation << ": " << error << std::endl;
    }
}
// LightManager.cpp
#include "LightManager.h"
#include "ShaderLoader.h"
#include "ModelLoader.h"

LightManager::LightManager()
    : pointLightsOn(true), directionalLightOn(true), spotLightOn(true)
{
    // Initialize 10 point lights with different positions and colors
    for (int i = 0; i < 10; ++i) {
        Light light;
        light.position = glm::vec3(100.0f * (i % 5), 50.0f + 20.0f * (i / 5), 100.0f * ((i + 2) % 5));
        light.color = glm::vec3(
            (i % 3 == 0) ? 1.0f : 0.0f,
            (i % 3 == 1) ? 1.0f : 0.0f,
            (i % 3 == 2) ? 1.0f : 0.0f
        ); // Cycle through red, green, blue
        light.linear = 0.09f;
        light.quadratic = 0.032f;
        pointLights.push_back(light);
    }

    // Initialize directional light
    directionalLight.direction = glm::vec3(-1.0f, -1.0f, -1.0f);
    directionalLight.color = glm::vec3(1.0f, 1.0f, 1.0f); // White color

    // Initialize spotlight
    spotLight.position = glm::vec3(0.0f);
    spotLight.direction = glm::vec3(0.0f, -1.0f, 0.0f);
    spotLight.color = glm::vec3(1.0f, 1.0f, 1.0f); // White color
    spotLight.cutOff = glm::cos(glm::radians(12.5f));
    spotLight.outerCutOff = glm::cos(glm::radians(15.0f));
}

void LightManager::initialize()
{
    ShaderLoader shaderLoader;
    lightShaderProgram = shaderLoader.CreateProgram(
        "Resources/Shaders/light_vertex_shader.vert",
        "Resources/Shaders/light_fragment_shader.frag"
    );

    // Load a cube model for light representation
    float vertices[] =
    {
        // positions          
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };

    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void LightManager::passLightData(GLuint shaderProgram, const glm::vec3& camPos, const glm::vec3& camDir)
{
    // Pass point lights
    glUniform1i(glGetUniformLocation(shaderProgram, "numLights"), pointLights.size());
    for (unsigned int i = 0; i < pointLights.size(); ++i)
    {
        std::string posName = "pointLights[" + std::to_string(i) + "].Position";
        std::string colorName = "pointLights[" + std::to_string(i) + "].Color";
        std::string linearName = "pointLights[" + std::to_string(i) + "].Linear";
        std::string quadraticName = "pointLights[" + std::to_string(i) + "].Quadratic";
        glUniform3fv(glGetUniformLocation(shaderProgram, posName.c_str()), 1, &pointLights[i].position[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, colorName.c_str()), 1, &pointLights[i].color[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, linearName.c_str()), pointLights[i].linear);
        glUniform1f(glGetUniformLocation(shaderProgram, quadraticName.c_str()), pointLights[i].quadratic);
    }

    // Pass directional light
    if (directionalLightOn) {
        glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.direction"), 1, &directionalLight.direction[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.color"), 1, &directionalLight.color[0]);
    }
    else {
        glm::vec3 zero = glm::vec3(0.0f);
        glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.direction"), 1, &zero[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "dirLight.color"), 1, &zero[0]);
    }

    // Pass spotlight
    if (spotLightOn) {
        spotLight.position = camPos;
        spotLight.direction = camDir;
        glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.position"), 1, &spotLight.position[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.direction"), 1, &spotLight.direction[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.color"), 1, &spotLight.color[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.cutOff"), spotLight.cutOff);
        glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.outerCutOff"), spotLight.outerCutOff);
    }
    else {
        glm::vec3 zero = glm::vec3(0.0f);
        glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.position"), 1, &zero[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.direction"), 1, &zero[0]);
        glUniform3fv(glGetUniformLocation(shaderProgram, "spotLight.color"), 1, &zero[0]);
        glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.cutOff"), 0.0f);
        glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.outerCutOff"), 0.0f);
    }
}

const LightManager::DirectionalLight& LightManager::getDirectionalLight() const {
    return directionalLight;
}

const LightManager::SpotLight& LightManager::getSpotLight() const {
    return spotLight;
}

std::vector<LightManager::Light> LightManager::getPointLights() const {
    return pointLights;
}

// Existing getters and setters
glm::vec3 LightManager::getPosition() const {
    return position;
}

glm::vec3 LightManager::getColor() const {
    return color;
}

void LightManager::setPosition(const glm::vec3& pos) {
    position = pos;
}

void LightManager::setColor(const glm::vec3& col) {
    color = col;
}

std::vector<LightManager::Light> LightManager::getLights() const {
    return { light1, light2 };
}

#include "LightManager.h"
#include "ShaderLoader.h"
#include "ModelLoader.h"

LightManager::LightManager()
    : pointLightsOn(true), directionalLightOn(true), spotLightOn(true) {
    light1.position = glm::vec3(150.0f, 50.0f, 0.0f);
    light1.color = glm::vec3(1.0f, 0.0f, 0.0f); // Red color

    light2.position = glm::vec3(150.0f, 50.0f, 150.0f);
    light2.color = glm::vec3(0.0f, 0.0f, 1.0f); // Blue color

    directionalLight.direction = glm::vec3(-1.0f, -1.0f, -1.0f);
    directionalLight.color = glm::vec3(1.0f, 1.0f, 1.0f); // White color

    spotLight.position = glm::vec3(0.0f);
    spotLight.direction = glm::vec3(0.0f);
    spotLight.color = glm::vec3(1.0f, 1.0f, 1.0f); // White color
    spotLight.cutOff = glm::cos(glm::radians(12.5f));
    spotLight.outerCutOff = glm::cos(glm::radians(15.0f));
}

void LightManager::initialize() {
    ShaderLoader shaderLoader;
    lightShaderProgram = shaderLoader.CreateProgram(
        "Resources/Shaders/light_vertex_shader.vert",
        "Resources/Shaders/light_fragment_shader.frag"
    );

    // Load the light sphere model
    ModelLoader sphereModelLoader("Resources/Models/Sphere_LowPoly.obj");
    sphereModelLoader.loadModel();
    lightSphereVertexCount = sphereModelLoader.getPositions().size(); 

    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);

    glBindVertexArray(lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, lightSphereVertexCount * sizeof(glm::vec3), sphereModelLoader.getPositions().data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void LightManager::passLightData(GLuint shaderProgram, const glm::vec3& camPos, const glm::vec3& camDir) {
    if (pointLightsOn) {
        passLightDataToShader(shaderProgram, light1, "lightPos1", "lightColor1");
        passLightDataToShader(shaderProgram, light2, "lightPos2", "lightColor2");
    }
    else {
       
        passLightDataToShader(shaderProgram, Light{ glm::vec3(0.0f), glm::vec3(0.0f) }, "lightPos1", "lightColor1");
        passLightDataToShader(shaderProgram, Light{ glm::vec3(0.0f), glm::vec3(0.0f) }, "lightPos2", "lightColor2");
    }

    if (directionalLightOn) {
        passDirectionalLightData(shaderProgram, directionalLight, "dirLightDir", "dirLightColor");
    }
    else {
      
        passDirectionalLightData(shaderProgram, DirectionalLight{ glm::vec3(0.0f), glm::vec3(0.0f) }, "dirLightDir", "dirLightColor");
    }

    if (spotLightOn) {
        spotLight.position = camPos;
        spotLight.direction = camDir;
        passSpotLightData(shaderProgram, spotLight, "spotLightPos", "spotLightDir", "spotLightColor", "spotLightCutOff", "spotLightOuterCutOff");
    }
    else {
   
        passSpotLightData(shaderProgram, SpotLight{ glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f), 0.0f, 0.0f }, "spotLightPos", "spotLightDir", "spotLightColor", "spotLightCutOff", "spotLightOuterCutOff");
    }
}

void LightManager::passLightDataToShader(GLuint shaderProgram, const Light& light, const std::string& lightPosName, const std::string& lightColorName) {
    glUniform3fv(glGetUniformLocation(shaderProgram, lightPosName.c_str()), 1, &light.position[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, lightColorName.c_str()), 1, &light.color[0]);
}

void LightManager::passDirectionalLightData(GLuint shaderProgram, const DirectionalLight& dirLight, const std::string& lightDirName, const std::string& lightColorName) {
    glUniform3fv(glGetUniformLocation(shaderProgram, lightDirName.c_str()), 1, &dirLight.direction[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, lightColorName.c_str()), 1, &dirLight.color[0]);
}

void LightManager::passSpotLightData(GLuint shaderProgram, const SpotLight& spotLight, const std::string& lightPosName, const std::string& lightDirName, const std::string& lightColorName, const std::string& cutOffName, const std::string& outerCutOffName) {
    glUniform3fv(glGetUniformLocation(shaderProgram, lightPosName.c_str()), 1, &spotLight.position[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, lightDirName.c_str()), 1, &spotLight.direction[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, lightColorName.c_str()), 1, &spotLight.color[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, cutOffName.c_str()), spotLight.cutOff);
    glUniform1f(glGetUniformLocation(shaderProgram, outerCutOffName.c_str()), spotLight.outerCutOff);
}

void LightManager::renderLightSpheres(const glm::mat4& viewProjectionMatrix) {
    if (pointLightsOn) {
        glUseProgram(lightShaderProgram);

        // Render the first light sphere
        glm::mat4 modelMatrix1 = glm::mat4(1.0f);
        modelMatrix1 = glm::translate(modelMatrix1, light1.position);
        modelMatrix1 = glm::scale(modelMatrix1, glm::vec3(10.0f)); // Scale the light sphere 

        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "viewProjection"), 1, GL_FALSE, &viewProjectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "model"), 1, GL_FALSE, &modelMatrix1[0][0]);
        glUniform3fv(glGetUniformLocation(lightShaderProgram, "lightColor"), 1, &light1.color[0]);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, lightSphereVertexCount);
        glBindVertexArray(0);

        // Render the second light sphere
        glm::mat4 modelMatrix2 = glm::mat4(1.0f);
        modelMatrix2 = glm::translate(modelMatrix2, light2.position);
        modelMatrix2 = glm::scale(modelMatrix2, glm::vec3(10.0f)); // Scale the light sphere

        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "viewProjection"), 1, GL_FALSE, &viewProjectionMatrix[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "model"), 1, GL_FALSE, &modelMatrix2[0][0]);
        glUniform3fv(glGetUniformLocation(lightShaderProgram, "lightColor"), 1, &light2.color[0]);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, lightSphereVertexCount);
        glBindVertexArray(0);
    }
}

glm::vec3 LightManager::getPosition() const
{
    return position;
}

glm::vec3 LightManager::getColor() const
{
    return color;
}

void LightManager::setPosition(const glm::vec3& pos)
{
    position = pos;
}

void LightManager::setColor(const glm::vec3& col)
{
    color = col;
}

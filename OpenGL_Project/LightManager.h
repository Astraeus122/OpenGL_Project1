#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include <glew.h>
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include <string>
#include <vector>

class LightManager {
public:
    struct Light {
        glm::vec3 position;
        glm::vec3 color;
        float linear;
        float quadratic;
    };

    struct DirectionalLight {
        glm::vec3 direction;
        glm::vec3 color;
    };

    struct SpotLight {
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 color;
        float cutOff;
        float outerCutOff;
    };

    LightManager();

    void initialize();
    void passLightData(GLuint shaderProgram, const glm::vec3& camPos, const glm::vec3& camDir);
    void passLightDataToShader(GLuint shaderProgram, const Light& light, const std::string& lightPosName, const std::string& lightColorName);
    void passDirectionalLightData(GLuint shaderProgram, const DirectionalLight& dirLight, const std::string& lightDirName, const std::string& lightColorName);
    void passSpotLightData(GLuint shaderProgram, const SpotLight& spotLight, const std::string& lightPosName, const std::string& lightDirName, const std::string& lightColorName, const std::string& cutOffName, const std::string& outerCutOffName);
    void renderLightSpheres(const glm::mat4& viewProjectionMatrix);
    void renderLightCubes(const glm::mat4& viewProjectionMatrix);

    // Getter for point lights
    std::vector<Light> getPointLights() const;

    // Getter for directional light
    const DirectionalLight& getDirectionalLight() const;

    // Getter for spotlight
    const SpotLight& getSpotLight() const;

    // Existing getters for position and color (if still needed)
    glm::vec3 getPosition() const;
    glm::vec3 getColor() const;
    void setPosition(const glm::vec3& pos);
    void setColor(const glm::vec3& col);

    std::vector<LightManager::Light> getLights() const;

private:
    std::vector<Light> pointLights;

    GLuint lightShaderProgram;
    GLuint lightVAO, lightVBO;
    unsigned int lightSphereVertexCount;

    // Directional and Spot Lights
    DirectionalLight directionalLight;
    SpotLight spotLight;

public:
    // Existing public members (if necessary)
    Light light1;
    Light light2;

    bool pointLightsOn;
    bool directionalLightOn;
    bool spotLightOn;

    glm::vec3 position;
    glm::vec3 color;
    float linear;
    float quadratic;
};

#endif 

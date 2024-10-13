#ifndef LIGHTMANAGER_H
#define LIGHTMANAGER_H

#include <glew.h>
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include <string>

class LightManager {
public:
    struct Light {
        glm::vec3 position;
        glm::vec3 color;
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

    Light light1;
    Light light2;
    DirectionalLight directionalLight;
    SpotLight spotLight;

    bool pointLightsOn;
    bool directionalLightOn;
    bool spotLightOn;

    glm::vec3 position;
    glm::vec3 color;
    float linear;
    float quadratic;

    glm::vec3 getPosition() const;
    glm::vec3 getColor() const;
    void setPosition(const glm::vec3& pos);
    void setColor(const glm::vec3& col);

private:
    GLuint lightVAO, lightVBO;
    GLuint lightShaderProgram;
    int lightSphereVertexCount; 
};

#endif 

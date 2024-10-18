#ifndef DEFERRED_SCENE_H
#define DEFERRED_SCENE_H

#include <vector>
#include "Dependencies/glm/glm.hpp"
#include "ModelLoader.h"
#include "LightManager.h"
#include "ShaderLoader.h"
#include "Plane.h" 

class DeferredScene
{
public:
    DeferredScene(unsigned int screenWidth, unsigned int screenHeight);
    void init(ShaderLoader& shaderLoader);

    // Update the geometryPass if necessary
    void geometryPass(const std::vector<ModelLoader*>& models, const glm::mat4& view, const glm::mat4& projection);

    // Updated lightingPass with four parameters
    void lightingPass(const std::vector<LightManager::Light>& pointLights,
        const LightManager::DirectionalLight& dirLight,
        const LightManager::SpotLight& spotLight,
        const glm::vec3& viewPos);

    void renderLights(const std::vector<LightManager::Light>& lights, const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int screenWidth, screenHeight;
    unsigned int gBuffer, gPosition, gNormal, gAlbedoSpec;
    unsigned int quadVAO, quadVBO;
    unsigned int lightVAO, lightVBO;
    unsigned int shaderGeometryPass, shaderLightingPass, shaderLightBox;

    // Add Plane
    Plane plane;

    void initGBuffer();
    void initScreenQuad();
    void initLightVAO();
};

#endif

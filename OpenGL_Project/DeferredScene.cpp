#include "DeferredScene.h"
#include "ShaderLoader.h"
#include <iostream>
#include "Dependencies/glm/gtc/type_ptr.hpp"

DeferredScene::DeferredScene(unsigned int screenWidth, unsigned int screenHeight)
    : screenWidth(screenWidth), screenHeight(screenHeight)
{
}

void DeferredScene::init(ShaderLoader& shaderLoader)
{
    initGBuffer();
    initScreenQuad();
    initLightVAO();

    // Initialize and load the plane
    plane.loadPlane();

    // Use ShaderLoader to create shader programs
    shaderGeometryPass = shaderLoader.CreateProgram("geometry_pass_vertex.txt", "geometry_pass_fragment.txt");
    shaderLightingPass = shaderLoader.CreateProgram("lighting_pass_vertex.txt", "lighting_pass_fragment.txt");
    shaderLightBox = shaderLoader.CreateProgram("lighting_box_vertex.txt", "lighting_box_fragment.txt");

    // Check for shader compilation/linking errors
    if (shaderGeometryPass == 0 || shaderLightingPass == 0 || shaderLightBox == 0)
    {
        std::cerr << "Error initializing one or more shaders in Deferred Rendering." << std::endl;
    }
}


void DeferredScene::initGBuffer()
{
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // Position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // Normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, screenWidth, screenHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // Color + Specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, screenWidth, screenHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    // Create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, screenWidth, screenHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "DeferredScene::initGBuffer - Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredScene::initScreenQuad()
{
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void DeferredScene::initLightVAO()
{
    float vertices[] = {
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
        -0.5f, -0.5f, -0.5f, // Fixed typo: -0 - 0.5f to -0.5f
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

void DeferredScene::geometryPass(const std::vector<ModelLoader*>& models, const glm::mat4& view, const glm::mat4& projection)
{
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderGeometryPass);

    // Render all 3D models
    for (const auto& model : models)
    {
        glm::mat4 modelMatrix = model->getModelMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass, "model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Set material properties (example: you might want to set these per model)
        glUniform3f(glGetUniformLocation(shaderGeometryPass, "objectAlbedo"), 0.5f, 0.0f, 0.0f); // Example color
        glUniform1f(glGetUniformLocation(shaderGeometryPass, "objectSpecular"), 1.0f); // Example specular

        model->render(shaderGeometryPass, view, projection);
    }

    // Render the plane
    glm::mat4 planeModel = plane.getModelMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass, "model"), 1, GL_FALSE, glm::value_ptr(planeModel));
    glUniform3f(glGetUniformLocation(shaderGeometryPass, "objectAlbedo"), 0.3f, 0.3f, 0.3f); // Gray color
    glUniform1f(glGetUniformLocation(shaderGeometryPass, "objectSpecular"), 0.5f); // Specular

    plane.render(shaderGeometryPass, view, projection);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeferredScene::lightingPass(const std::vector<LightManager::Light>& pointLights,
    const LightManager::DirectionalLight& dirLight,
    const LightManager::SpotLight& spotLight,
    const glm::vec3& viewPos)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderLightingPass);

    // Pass point lights
    glUniform1i(glGetUniformLocation(shaderLightingPass, "numLights"), pointLights.size());
    for (unsigned int i = 0; i < pointLights.size(); ++i)
    {
        std::string posName = "pointLights[" + std::to_string(i) + "].Position";
        std::string colorName = "pointLights[" + std::to_string(i) + "].Color";
        std::string linearName = "pointLights[" + std::to_string(i) + "].Linear";
        std::string quadraticName = "pointLights[" + std::to_string(i) + "].Quadratic";
        glUniform3fv(glGetUniformLocation(shaderLightingPass, posName.c_str()), 1, glm::value_ptr(pointLights[i].position));
        glUniform3fv(glGetUniformLocation(shaderLightingPass, colorName.c_str()), 1, glm::value_ptr(pointLights[i].color));
        glUniform1f(glGetUniformLocation(shaderLightingPass, linearName.c_str()), pointLights[i].linear);
        glUniform1f(glGetUniformLocation(shaderLightingPass, quadraticName.c_str()), pointLights[i].quadratic);
    }

    // Pass directional light
    if (dirLight.color != glm::vec3(0.0f)) {
        glUniform3fv(glGetUniformLocation(shaderLightingPass, "dirLight.direction"), 1, glm::value_ptr(dirLight.direction));
        glUniform3fv(glGetUniformLocation(shaderLightingPass, "dirLight.color"), 1, glm::value_ptr(dirLight.color));
    }
    else {
        glm::vec3 zero = glm::vec3(0.0f);
        glUniform3fv(glGetUniformLocation(shaderLightingPass, "dirLight.direction"), 1, glm::value_ptr(zero));
        glUniform3fv(glGetUniformLocation(shaderLightingPass, "dirLight.color"), 1, glm::value_ptr(zero));
    }

    // Pass spotlight
    if (spotLight.color != glm::vec3(0.0f)) {
        glUniform3fv(glGetUniformLocation(shaderLightingPass, "spotLight.position"), 1, glm::value_ptr(spotLight.position));
        glUniform3fv(glGetUniformLocation(shaderLightingPass, "spotLight.direction"), 1, glm::value_ptr(spotLight.direction));
        glUniform3fv(glGetUniformLocation(shaderLightingPass, "spotLight.color"), 1, glm::value_ptr(spotLight.color));
        glUniform1f(glGetUniformLocation(shaderLightingPass, "spotLight.cutOff"), spotLight.cutOff);
        glUniform1f(glGetUniformLocation(shaderLightingPass, "spotLight.outerCutOff"), spotLight.outerCutOff);
    }
    else {
        glm::vec3 zero = glm::vec3(0.0f);
        glUniform3fv(glGetUniformLocation(shaderLightingPass, "spotLight.position"), 1, glm::value_ptr(zero));
        glUniform3fv(glGetUniformLocation(shaderLightingPass, "spotLight.direction"), 1, glm::value_ptr(zero));
        glUniform3fv(glGetUniformLocation(shaderLightingPass, "spotLight.color"), 1, glm::value_ptr(zero));
        glUniform1f(glGetUniformLocation(shaderLightingPass, "spotLight.cutOff"), 0.0f);
        glUniform1f(glGetUniformLocation(shaderLightingPass, "spotLight.outerCutOff"), 0.0f);
    }

    glUniform3fv(glGetUniformLocation(shaderLightingPass, "viewPos"), 1, glm::value_ptr(viewPos));

    // Bind G-buffer textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

    // Render screen quad
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void DeferredScene::renderLights(const std::vector<LightManager::Light>& lights, const glm::mat4& view, const glm::mat4& projection)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(shaderLightBox);
    for (const auto& light : lights)
    {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, light.position);
        model = glm::scale(model, glm::vec3(0.2f));

        glUniformMatrix4fv(glGetUniformLocation(shaderLightBox, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderLightBox, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderLightBox, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniform3fv(glGetUniformLocation(shaderLightBox, "lightColor"), 1, glm::value_ptr(light.color));

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
    glDisable(GL_BLEND);
}

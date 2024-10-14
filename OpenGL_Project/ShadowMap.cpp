#include "ShadowMap.h"

ShadowMap::ShadowMap() : depthMapFBO(0), depthMap(0), shadowWidth(4096), shadowHeight(4096) {}


ShadowMap::~ShadowMap()
{
    glDeleteFramebuffers(1, &depthMapFBO);
    glDeleteTextures(1, &depthMap);
}

void ShadowMap::init(unsigned int width, unsigned int height)
{
    shadowWidth = width;
    shadowHeight = height;

    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    // Use linear filtering for smoother shadows
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Set texture wrapping to clamp to border to avoid shadow artifacts
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void ShadowMap::setLightPosition(const glm::vec3& lightDir)
{
    glm::mat4 lightProjection, lightView;
    float near_plane = 1.0f, far_plane = 5000.0f; 

    lightProjection = glm::ortho(-3000.0f, 3000.0f, -3000.0f, 3000.0f, near_plane, far_plane);
    lightView = glm::lookAt(-lightDir * 3000.0f, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;
}

void ShadowMap::bind()
{
    glViewport(0, 0, shadowWidth, shadowHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLuint ShadowMap::getDepthMap() const
{
    return depthMap;
}

glm::mat4 ShadowMap::getLightSpaceMatrix() const
{
    return lightSpaceMatrix;
}
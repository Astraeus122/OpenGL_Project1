#ifndef SHADOW_MAP_H
#define SHADOW_MAP_H

#include <glew.h>
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

class ShadowMap
{
public:
    ShadowMap();
    ~ShadowMap();

    void init(unsigned int width, unsigned int height);
    void bind();
    void unbind();

    GLuint getDepthMap() const;
    glm::mat4 getLightSpaceMatrix() const;
    void setLightPosition(const glm::vec3& lightDir);

private:
    GLuint depthMapFBO;
    GLuint depthMap;
    unsigned int shadowWidth, shadowHeight;
    glm::mat4 lightSpaceMatrix;
};

#endif // SHADOW_MAP_H

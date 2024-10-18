#ifndef SKYBOX_H
#define SKYBOX_H

#include <vector>
#include <string>
#include <glew.h>
#include "Dependencies/glm/glm.hpp"

class Skybox {
public:
    Skybox(const std::vector<std::string>& faces);
    void render(const glm::mat4& viewProjectionMatrix);
    GLuint shaderProgram;

private:
    GLuint cubemapTexture;
    GLuint VAO, VBO;

    GLuint loadCubemap(const std::vector<std::string>& faces);
    void initSkybox();
};

#endif 
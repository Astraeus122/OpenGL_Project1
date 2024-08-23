#ifndef INSTANCEDRENDERER_H
#define INSTANCEDRENDERER_H

#include <glew.h>
#include <vector>
#include "Dependencies/glm/glm.hpp"
#include "Texture.h"

class InstancedRenderer {
public:
    InstancedRenderer(const std::vector<glm::vec3>& positions, const std::vector<glm::vec2>& texCoords, const std::vector<glm::vec3>& normals, const std::string& texturePath, int instanceCount);
    ~InstancedRenderer();
    void initialize();
    void render(GLuint shaderProgram, const glm::mat4& viewProjectionMatrix);

private:
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    Texture texture;
    int instanceCount;
    GLuint VAO, VBO, texCoordVBO, normalVBO, instanceVBO;
};

#endif 
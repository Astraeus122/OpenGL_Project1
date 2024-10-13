#ifndef INSTANCEDRENDERER_H
#define INSTANCEDRENDERER_H

#include <glew.h>
#include <vector>
#include <string>
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/tiny_obj_loader.h"
#include "Texture.h"

class InstancedRenderer
{
public:
    InstancedRenderer(const std::string& modelPath, const std::string& texturePath, int instanceCount);
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

    void loadModelData(const std::string& modelPath);
};

#endif 

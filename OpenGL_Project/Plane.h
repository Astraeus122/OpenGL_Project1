#ifndef PLANE_H
#define PLANE_H

#include "ModelLoader.h"

class Plane : public ModelLoader
{
public:
    Plane();
    ~Plane() = default;
    void loadPlane();
    void render(GLuint shaderProgram, const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix);
};

#endif

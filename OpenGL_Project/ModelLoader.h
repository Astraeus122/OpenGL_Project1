#pragma once

#include <glew.h>
#include <vector>
#include <string>
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/ext/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"

class ModelLoader {
public:
    ModelLoader(const std::string& modelPath);
    ~ModelLoader();

    void loadModel();
    void render(GLuint shaderProgram, const glm::mat4& view, const glm::mat4& projection);

    // Transformation Methods
    void rotate(float radians, const glm::vec3& axis);
    void translate(const glm::vec3& offset);
    void scale(const glm::vec3& scaleFactor);
    void resetTransformation();
    glm::mat4 getModelMatrix() const { return modelMatrix; }
    void setModelMatrix(const glm::mat4& matrix) { modelMatrix = matrix; }

    // Getters for positions, texcoords, normals
    const std::vector<glm::vec3>& getPositions() const;
    const std::vector<glm::vec2>& getTexCoords() const;
    const std::vector<glm::vec3>& getNormals() const;

private:
    std::string modelPath;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

    glm::mat4 modelMatrix = glm::mat4(1.0f); // Transformation matrix

    GLuint VAO, VBO, EBO; // OpenGL handles for rendering

    void setupMesh();
};

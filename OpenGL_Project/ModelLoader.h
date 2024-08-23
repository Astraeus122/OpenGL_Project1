#pragma once
#include <glew.h>
#include <vector>
#include <string>
#include "Dependencies/tiny_obj_loader.h"
#include "Dependencies/glm/glm.hpp"

class ModelLoader {
public:
    ModelLoader(const std::string& modelPath);
    ~ModelLoader();

    void loadModel();

    const std::vector<glm::vec3>& getPositions() const;
    const std::vector<glm::vec2>& getTexCoords() const;
    const std::vector<glm::vec3>& getNormals() const;

private:
    std::string modelPath;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec2> texCoords;
    std::vector<glm::vec3> normals;
};

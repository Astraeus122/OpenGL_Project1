#include "ModelLoader.h"
#include <iostream>

ModelLoader::ModelLoader(const std::string& modelPath) : modelPath(modelPath) {}

ModelLoader::~ModelLoader() {}

void ModelLoader::loadModel() {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str());
    if (!ret) {
        std::cerr << "Failed to load model: " << warn << err << std::endl;
        return;
    }

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            positions.push_back(glm::vec3(attrib.vertices[3 * index.vertex_index + 0], attrib.vertices[3 * index.vertex_index + 1], attrib.vertices[3 * index.vertex_index + 2]));
            texCoords.push_back(glm::vec2(attrib.texcoords[2 * index.texcoord_index + 0], 1.0f - attrib.texcoords[2 * index.texcoord_index + 1]));
            normals.push_back(glm::vec3(attrib.normals[3 * index.normal_index + 0], attrib.normals[3 * index.normal_index + 1], attrib.normals[3 * index.normal_index + 2]));
        }
    }
}

const std::vector<glm::vec3>& ModelLoader::getPositions() const {
    return positions;
}

const std::vector<glm::vec2>& ModelLoader::getTexCoords() const {
    return texCoords;
}

const std::vector<glm::vec3>& ModelLoader::getNormals() const {
    return normals;
}
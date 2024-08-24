#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "PerlinNoiseScene.h"
#include <iostream>
#include <fstream>
#include <ctime>
#include "Dependencies/stb_image.h"
#include "Dependencies/glm/gtc/type_ptr.hpp"
#include "Dependencies/stb_image_write.h"
#include <numeric>
#include <random>

PerlinNoiseScene::PerlinNoiseScene(ShaderLoader& shaderLoader, Camera& camera)
    : m_shaderLoader(shaderLoader), m_camera(camera), m_time(0.0f) {
    m_terrainShaderProgram = m_shaderLoader.CreateProgram("perlin_vertex_shader.txt", "perlin_fragment_shader.txt");
    m_2dNoiseShaderProgram = m_shaderLoader.CreateProgram("2d_perlin_vertex_shader.txt", "2d_perlin_fragment_shader.txt");
    initializeNoise();
}

void PerlinNoiseScene::initialize() {
    generateTerrainMesh();
    loadTerrainTexture();
    setup2DQuad();
    generate2DNoiseTexture();
}

void PerlinNoiseScene::initializeNoise() {
    p.resize(256);
    std::iota(p.begin(), p.end(), 0);
    std::default_random_engine engine((unsigned int)time(nullptr));
    std::shuffle(p.begin(), p.end(), engine);
    p.insert(p.end(), p.begin(), p.end());
}

double PerlinNoiseScene::fade(double t) {
    return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoiseScene::lerp(double t, double a, double b) {
    return a + t * (b - a);
}

double PerlinNoiseScene::grad(int hash, double x, double y, double z) {
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

double PerlinNoiseScene::noise(double x, double y, double z) {
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;

    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;

    double res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
        grad(p[BA], x - 1, y, z)),
        lerp(u, grad(p[AB], x, y - 1, z),
            grad(p[BB], x - 1, y - 1, z))),
        lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
            grad(p[BA + 1], x - 1, y, z - 1)),
            lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                grad(p[BB + 1], x - 1, y - 1, z - 1))));
    return (res + 1.0) / 2.0;
}

void PerlinNoiseScene::generateAndSavePerlinNoiseImage(const std::string& jpgFilePath, const std::string& rawFilePath) {
    const unsigned int width = 512, height = 512;
    std::vector<unsigned char> image(width * height * 3);

    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            double noiseValue = noise(x / (double)width, y / (double)height, 0.0);
            unsigned char color = static_cast<unsigned char>(noiseValue * 255);
            image[3 * (y * width + x) + 0] = color;
            image[3 * (y * width + x) + 1] = color;
            image[3 * (y * width + x) + 2] = color;
        }
    }

    if (!stbi_write_jpg(jpgFilePath.c_str(), width, height, 3, image.data(), 100)) {
        std::cerr << "Failed to write image: " << jpgFilePath << std::endl;
    }

    std::ofstream rawFile(rawFilePath, std::ios::out | std::ios::binary);
    if (rawFile.is_open()) {
        rawFile.write(reinterpret_cast<char*>(image.data()), image.size());
        rawFile.close();
    }
    else {
        std::cerr << "Failed to write raw file: " << rawFilePath << std::endl;
    }
}

void PerlinNoiseScene::generateTerrainMesh() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    const int resolution = 128;
    const float size = 100.0f;

    for (int z = 0; z < resolution; ++z) {
        for (int x = 0; x < resolution; ++x) {
            float xPos = (float)x / (resolution - 1) * size - size / 2;
            float zPos = (float)z / (resolution - 1) * size - size / 2;
            float yPos = noise(xPos * 0.1, zPos * 0.1, 0) * 10.0f;

            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
            vertices.push_back((float)x / (resolution - 1));
            vertices.push_back((float)z / (resolution - 1));
        }
    }

    for (int z = 0; z < resolution - 1; ++z) {
        for (int x = 0; x < resolution - 1; ++x) {
            unsigned int topLeft = z * resolution + x;
            unsigned int topRight = topLeft + 1;
            unsigned int bottomLeft = (z + 1) * resolution + x;
            unsigned int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);
            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    glGenVertexArrays(1, &m_terrainVAO);
    glGenBuffers(1, &m_terrainVBO);
    glGenBuffers(1, &m_terrainEBO);

    glBindVertexArray(m_terrainVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void PerlinNoiseScene::loadTerrainTexture() {
    generateAndSavePerlinNoiseImage("perlin_noise_texture.jpg", "perlin_noise_heightmap.raw");

    int width, height, nrChannels;
    unsigned char* data = stbi_load("perlin_noise_texture.jpg", &width, &height, &nrChannels, 0);

    if (data) {
        glGenTextures(1, &m_terrainTexture);
        glBindTexture(GL_TEXTURE_2D, m_terrainTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cerr << "Failed to load texture" << std::endl;
    }
}

void PerlinNoiseScene::setup2DQuad() {
    float quadVertices[] = {
        // positions   // texCoords
        0.5f,  1.0f,  0.0f, 1.0f,
        0.5f,  0.5f,  0.0f, 0.0f,
        1.0f,  0.5f,  1.0f, 0.0f,

        0.5f,  1.0f,  0.0f, 1.0f,
        1.0f,  0.5f,  1.0f, 0.0f,
        1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_2dQuadVAO);
    glGenBuffers(1, &m_2dQuadVBO);
    glBindVertexArray(m_2dQuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_2dQuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
}

void PerlinNoiseScene::generate2DNoiseTexture() {
    const unsigned int width = 256, height = 256;
    std::vector<unsigned char> noiseData(width * height * 3);

    glGenTextures(1, &m_2dNoiseTexture);
    glBindTexture(GL_TEXTURE_2D, m_2dNoiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void PerlinNoiseScene::update(float deltaTime) {
    m_time += deltaTime;
}

void PerlinNoiseScene::render() {
    // Render 3D terrain
    glEnable(GL_DEPTH_TEST);
    glUseProgram(m_terrainShaderProgram);

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = m_camera.getViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glUniformMatrix4fv(glGetUniformLocation(m_terrainShaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(m_terrainShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(m_terrainShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_terrainTexture);
    glUniform1i(glGetUniformLocation(m_terrainShaderProgram, "perlinTexture"), 0);

    glBindVertexArray(m_terrainVAO);
    glDrawElements(GL_TRIANGLES, (128 - 1) * (128 - 1) * 6, GL_UNSIGNED_INT, 0);

    // Render 2D animated noise quad
    glDisable(GL_DEPTH_TEST);
    glUseProgram(m_2dNoiseShaderProgram);

    // Update the time uniform
    glUniform1f(glGetUniformLocation(m_2dNoiseShaderProgram, "time"), m_time);

    glBindVertexArray(m_2dQuadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
}

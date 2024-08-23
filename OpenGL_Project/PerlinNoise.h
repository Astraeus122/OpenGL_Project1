#pragma once
#include <vector>
#include <string>
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/stb_image_write.h"

class PerlinNoise {
public:
    PerlinNoise();
    PerlinNoise(unsigned int seed);
    double noise(double x, double y, double z);
    void generateAndSavePerlinNoiseImage(const std::string& jpgFilePath, const std::string& rawFilePath);

private:
    std::vector<int> p;
    double fade(double t);
    double lerp(double t, double a, double b);
    double grad(int hash, double x, double y, double z);
};
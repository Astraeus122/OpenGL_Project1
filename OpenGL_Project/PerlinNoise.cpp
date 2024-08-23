#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "PerlinNoise.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <numeric>
#include <random>
#include <ctime>
#include <algorithm>
#include <limits>

// Constructs and initializes a Perlin noise generator with a random seed
PerlinNoise::PerlinNoise() : PerlinNoise((unsigned int)time(nullptr)) {}

// Constructs a Perlin noise generator with the given seed
PerlinNoise::PerlinNoise(unsigned int seed)
{
    p.resize(256);

    // Fills p with values from 0 to 255
    std::iota(p.begin(), p.end(), 0);

    // Shuffle the array using the provided seed
    std::default_random_engine engine(seed);
    std::shuffle(p.begin(), p.end(), engine);

    // Duplicate the permutation vector
    p.insert(p.end(), p.begin(), p.end());
}

// Helper function to blend noise values
double PerlinNoise::fade(double t)
{
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Helper function for linear interpolation
double PerlinNoise::lerp(double t, double a, double b)
{
    return a + t * (b - a);
}

// Helper function to hash an integer using the permutation vector
double PerlinNoise::grad(int hash, double x, double y, double z)
{
    int h = hash & 15;
    double u = h < 8 ? x : y;
    double v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}

// Compute Perlin noise at coordinates x, y, z
double PerlinNoise::noise(double x, double y, double z)
{
    // Find the unit cube that contains the point
    int X = (int)floor(x) & 255;
    int Y = (int)floor(y) & 255;
    int Z = (int)floor(z) & 255;

    // Find relative x, y, z of point in cube
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

    // Compute fade curves for x, y, z
    double u = fade(x);
    double v = fade(y);
    double w = fade(z);

    // Hash coordinates of the 8 cube corners
    int A = p[X] + Y;
    int AA = p[A] + Z;
    int AB = p[A + 1] + Z;
    int B = p[X + 1] + Y;
    int BA = p[B] + Z;
    int BB = p[B + 1] + Z;

    // And add blended results from 8 corners of the cube
    double res = lerp(w, lerp(v, lerp(u, grad(p[AA], x, y, z),
        grad(p[BA], x - 1, y, z)),
        lerp(u, grad(p[AB], x, y - 1, z),
            grad(p[BB], x - 1, y - 1, z))),
        lerp(v, lerp(u, grad(p[AA + 1], x, y, z - 1),
            grad(p[BA + 1], x - 1, y, z - 1)),
            lerp(u, grad(p[AB + 1], x, y - 1, z - 1),
                grad(p[BB + 1], x - 1, y - 1, z - 1))));
    return (res + 1.0) / 2.0; // Map the result to the [0, 1] interval
}

void PerlinNoise::generateAndSavePerlinNoiseImage(const std::string& jpgFilePath, const std::string& rawFilePath) {
    const unsigned int width = 512, height = 512;
    std::vector<unsigned char> image(width * height * 3); // 3 channels for RGB

    // Variables for debugging the range of noise values
    double minNoiseValue = std::numeric_limits<double>::max();
    double maxNoiseValue = -std::numeric_limits<double>::max();

    // Generate Perlin noise data and fill the image array
    for (unsigned int y = 0; y < height; ++y) {
        for (unsigned int x = 0; x < width; ++x) {
            double noiseValue = this->noise(x / (double)width, y / (double)height, 0.0);
            // Update the min and max noise values
            minNoiseValue = std::min(minNoiseValue, noiseValue);
            maxNoiseValue = std::max(maxNoiseValue, noiseValue);

            unsigned char color = static_cast<unsigned char>(noiseValue * 255);
            image[3 * (y * width + x) + 0] = color; // Red
            image[3 * (y * width + x) + 1] = color; // Green
            image[3 * (y * width + x) + 2] = color; // Blue
        }
    }

    // Save the image as a JPG file
    if (!stbi_write_jpg(jpgFilePath.c_str(), width, height, 3, image.data(), 100)) {
        std::cerr << "Failed to write image: " << jpgFilePath << std::endl;
    }

    // Save the raw data for the heightmap
    std::ofstream rawFile(rawFilePath, std::ios::out | std::ios::binary);
    if (rawFile.is_open()) {
        rawFile.write(reinterpret_cast<char*>(image.data()), image.size());
        rawFile.close();
    }
    else {
        std::cerr << "Failed to write raw file: " << rawFilePath << std::endl;
    }
    std::cout << std::endl;
}

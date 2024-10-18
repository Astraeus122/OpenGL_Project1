#include "ParticleSystem.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <random>
#include "Dependencies/glm/gtc/type_ptr.hpp"

ParticleSystem::ParticleSystem(const std::string& computeShaderPath, const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
    std::string computeShaderSource = readShaderSourceFromFile(computeShaderPath);
    std::string vertexShaderSource = readShaderSourceFromFile(vertexShaderPath);
    std::string fragmentShaderSource = readShaderSourceFromFile(fragmentShaderPath);

    computeShaderProgram = createShaderProgram(computeShaderSource.c_str(), nullptr, nullptr);
    if (!computeShaderProgram)
    {
        std::cerr << "Compute Shader Program creation failed!" << std::endl;
    }

    renderShaderProgram = createShaderProgram(nullptr, vertexShaderSource.c_str(), fragmentShaderSource.c_str());
    if (!renderShaderProgram)
    {
        std::cerr << "Render Shader Program creation failed!" << std::endl;
    }
}

ParticleSystem::~ParticleSystem()
{
    glDeleteProgram(computeShaderProgram);
    glDeleteProgram(renderShaderProgram);
    glDeleteBuffers(1, &particleBuffer);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &particleBuffer);
}

void ParticleSystem::init()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &particleBuffer); // Use particleBuffer as both VBO and SSBO

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, particleBuffer);
    glBufferData(GL_ARRAY_BUFFER, maxParticles * sizeof(Particle), nullptr, GL_DYNAMIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)0);

    // Color attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Particle), (void*)(offsetof(Particle, color)));

    glBindVertexArray(0);

    // Bind particleBuffer as SSBO
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);

    // Initialize particle data
    std::vector<Particle> initialParticles(maxParticles);
    for (int i = 0; i < maxParticles; ++i)
    {
        initialParticles[i].pos = glm::vec4(0.0f, -10.0f, 0.0f, 1.0f);
        initialParticles[i].vel = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        initialParticles[i].color = glm::vec4(1.0f); // White color
    }
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, maxParticles * sizeof(Particle), initialParticles.data());

    // Bind particleBuffer to SSBO binding point 0
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBuffer);
}

void ParticleSystem::update(float deltaTime)
{
    glUseProgram(computeShaderProgram);
    glUniform1f(glGetUniformLocation(computeShaderProgram, "uDeltaTime"), deltaTime);
    glUniform1ui(glGetUniformLocation(computeShaderProgram, "uMaxParticles"), maxParticles);
    glDispatchCompute((GLuint)(maxParticles / 256) + 1, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

void ParticleSystem::render(const glm::mat4& view, const glm::mat4& projection)
{
    glUseProgram(renderShaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(renderShaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(renderShaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(vao);
    glDrawArrays(GL_POINTS, 0, maxParticles);
    glBindVertexArray(0);
}

std::string ParticleSystem::readShaderSourceFromFile(const std::string& shaderFilePath)
{
    std::string shaderCode;
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try
    {
        shaderFile.open(shaderFilePath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        shaderCode = shaderStream.str();
    }
    catch (const std::ifstream::failure& e)
    {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << shaderFilePath << std::endl;
    }
    return shaderCode;
}

GLuint ParticleSystem::compileShader(const char* shaderSource, GLenum shaderType)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint ParticleSystem::createShaderProgram(const char* computeShaderSource, const char* vertexShaderSource, const char* fragmentShaderSource)
{
    GLuint computeShader, vertexShader, fragmentShader;

    if (computeShaderSource)
    {
        computeShader = compileShader(computeShaderSource, GL_COMPUTE_SHADER);
    }
    if (vertexShaderSource)
    {
        vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    }
    if (fragmentShaderSource)
    {
        fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    }

    GLuint shaderProgram = glCreateProgram();
    if (computeShaderSource)
    {
        glAttachShader(shaderProgram, computeShader);
    }
    if (vertexShaderSource)
    {
        glAttachShader(shaderProgram, vertexShader);
    }
    if (fragmentShaderSource)
    {
        glAttachShader(shaderProgram, fragmentShader);
    }
    glLinkProgram(shaderProgram);

    int success;
    char infoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    if (computeShaderSource)
    {
        glDeleteShader(computeShader);
    }
    if (vertexShaderSource)
    {
        glDeleteShader(vertexShader);
    }
    if (fragmentShaderSource)
    {
        glDeleteShader(fragmentShader);
    }

    return shaderProgram;
}

void ParticleSystem::triggerFirework(const glm::vec3& position, const glm::vec4& color) {
    // Number of particles per firework
    int particlesPerFirework = 25000; // 100000 / 4

    std::vector<Particle> fireworkParticles(particlesPerFirework);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * glm::pi<float>());
    std::uniform_real_distribution<float> phiDist(0.0f, glm::pi<float>());
    std::uniform_real_distribution<float> speedDist(5.0f, 15.0f);

    for (int i = 0; i < particlesPerFirework; ++i)
    {
        // Initial position at the spawn position
        fireworkParticles[i].pos = glm::vec4(position, 1.0f);

        // Random direction for explosion
        float theta = angleDist(gen);
        float phi = phiDist(gen);
        float speed = speedDist(gen);

        float vx = speed * sin(phi) * cos(theta);
        float vy = speed * cos(phi);
        float vz = speed * sin(phi) * sin(theta);

        fireworkParticles[i].vel = glm::vec4(vx, vy, vz, 0.0f);

        // Assign the color of the firework
        fireworkParticles[i].color = color;
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBuffer);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, particlesPerFirework * sizeof(Particle), fireworkParticles.data());
}

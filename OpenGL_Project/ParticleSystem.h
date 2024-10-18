#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include "Dependencies/glm/glm.hpp"
#include <vector>
#include <glew.h>
#include <string>
#include <mutex>

class ParticleSystem
{
public:
    ParticleSystem(const std::string& computeShaderPath, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
    ~ParticleSystem();

    void init();
    void update(float deltaTime);
    void render(const glm::mat4& view, const glm::mat4& projection);

    GLuint renderShaderProgram;
    GLuint vao;
    GLuint particleBuffer;
    int maxParticles = 100000;

    struct Particle {
        glm::vec4 pos;
        glm::vec4 vel;
        glm::vec4 color;
    };

    void triggerFirework(const glm::vec3& position, const glm::vec4& color);

private:
    GLuint computeShaderProgram;
    GLuint compileShader(const char* shaderSource, GLenum shaderType);
    GLuint createShaderProgram(const char* computeShaderSource, const char* vertexShaderSource, const char* fragmentShaderSource);
    std::string readShaderSourceFromFile(const std::string& shaderFilePath);

    std::mutex fireworkMutex;
    int currentIndex = 0;     // Tracks the next available slot in the buffer
};

#endif

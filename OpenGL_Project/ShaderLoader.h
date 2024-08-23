#ifndef SHADERLOADER_H
#define SHADERLOADER_H

#include <glew.h> 
#include <string>

class ShaderLoader {
public:
    ShaderLoader();
    ~ShaderLoader();

    GLuint CreateShader(GLenum shaderType, const char* shaderName);
    GLuint CreateProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename);

private:
    std::string ReadShaderFile(const char* filename);
    void PrintErrorDetails(bool isShader, GLuint id, const char* name);
};

#endif

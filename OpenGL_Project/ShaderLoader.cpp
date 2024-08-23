#include "ShaderLoader.h"
#include <iostream>
#include <fstream>
#include <vector>

ShaderLoader::ShaderLoader() {}
ShaderLoader::~ShaderLoader() {}

GLuint ShaderLoader::CreateShader(GLenum shaderType, const char* shaderName) {
    std::string shaderSourceCode = ReadShaderFile(shaderName);
    GLuint shaderID = glCreateShader(shaderType);
    const char* shader_code_ptr = shaderSourceCode.c_str();
    const int shader_code_size = (int)shaderSourceCode.size();

    glShaderSource(shaderID, 1, &shader_code_ptr, &shader_code_size);
    glCompileShader(shaderID);

    int compile_result = 0;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &compile_result);
    if (compile_result == GL_FALSE) {
        PrintErrorDetails(true, shaderID, shaderName);
        return 0;
    }
    return shaderID;
}

GLuint ShaderLoader::CreateProgram(const char* vertexShaderFilename, const char* fragmentShaderFilename) {
    GLuint vertexShaderID = CreateShader(GL_VERTEX_SHADER, vertexShaderFilename);
    GLuint fragmentShaderID = CreateShader(GL_FRAGMENT_SHADER, fragmentShaderFilename);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShaderID);
    glAttachShader(program, fragmentShaderID);
    glLinkProgram(program);

    int link_result = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &link_result);
    if (link_result == GL_FALSE) {
        std::string programName = std::string(vertexShaderFilename) + fragmentShaderFilename;
        PrintErrorDetails(false, program, programName.c_str());
        return 0;
    }
    return program;
}

std::string ShaderLoader::ReadShaderFile(const char* filename) {
    std::ifstream file(filename, std::ios::in);
    std::string shaderCode;

    if (!file.good()) {
        std::cerr << "Cannot read file: " << filename << std::endl;
        return "";
    }

    file.seekg(0, std::ios::end);
    shaderCode.resize((unsigned int)file.tellg());
    file.seekg(0, std::ios::beg);
    file.read(&shaderCode[0], shaderCode.size());
    file.close();
    return shaderCode;
}

void ShaderLoader::PrintErrorDetails(bool isShader, GLuint id, const char* name) {
    int infoLogLength = 0;
    (isShader == true) ? glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength) : glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<char> log(infoLogLength);
    (isShader == true) ? glGetShaderInfoLog(id, infoLogLength, NULL, &log[0]) : glGetProgramInfoLog(id, infoLogLength, NULL, &log[0]);
    std::cerr << "Error compiling " << ((isShader == true) ? "shader" : "program") << ": " << name << std::endl;
    std::cerr << &log[0] << std::endl;
}

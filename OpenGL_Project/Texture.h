
#ifndef TEXTURE_H
#define TEXTURE_H

#include <glew.h>
#include <string>


class Texture {
public:
    Texture(const std::string& filePath);
    ~Texture();

    void bind();
    void unbind();

private:
    GLuint textureID;
    int width, height, channels;
};

#endif

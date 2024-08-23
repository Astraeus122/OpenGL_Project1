#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords; // Texture coordinate attribute

out vec3 Normal;
out vec2 TexCoords;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // Pass the normals and texture coordinates to the fragment shader
    Normal = aNormal;
    TexCoords = aTexCoords;

    // Calculate the fragment position vector in world space for lighting calculations
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Calculate the final position of the vertex
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

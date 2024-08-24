#version 330 core

layout (location = 0) in vec3 aPos; // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float maxHeight;

out vec3 FragPos; // Position of the fragment for lighting
out vec3 Normal; // Normal of the fragment for lighting
out vec2 TexCoords; // Texture coordinates for blending based on height

void main() 
{
    // Compute the vertex position in world space
    vec4 worldPosition = model * vec4(aPos, 1.0);
    FragPos = vec3(worldPosition); // Pass this to the fragment shader

    // Compute the normal in world space
    Normal = mat3(transpose(inverse(model))) * aNormal; 

    // Compute texture coordinates based on the height
    TexCoords = vec2(aPos.x, aPos.z / maxHeight); 

    // Transform the vertex position to clip space
    gl_Position = projection * view * worldPosition; 
}
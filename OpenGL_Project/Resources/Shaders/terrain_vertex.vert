// terrain_vertex_shader.glsl
#version 330 core

layout (location = 0) in vec3 aPos; // Vertex position
layout (location = 1) in vec3 aNormal; // Vertex normal

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float maxHeight;

out vec3 FragPos; // Position of the fragment for lighting
out vec3 Normal; // Normal of the fragment for lighting
out float Height; // Pass height as a float to the fragment shader

void main() 
{
    // Compute the vertex position in world space
    vec4 worldPosition = model * vec4(aPos, 1.0);
    FragPos = vec3(worldPosition); // Pass this to the fragment shader

    // Compute the normal in world space
    Normal = mat3(transpose(inverse(model))) * aNormal; 

    // Compute height based on y-coordinate
    Height = aPos.y / maxHeight; // Normalize height based on maxHeight

    // Transform the vertex position to clip space
    gl_Position = projection * view * worldPosition; 
}


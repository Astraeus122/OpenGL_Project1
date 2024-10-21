#version 430 core

layout(location = 0) in vec3 inPosition;  // Vertex position input

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 vPosition;  // Pass position to tessellation shaders

void main() {
    vPosition = vec3(modelMatrix * vec4(inPosition, 1.0));  // Pass model-transformed position
    gl_Position = projectionMatrix * viewMatrix * vec4(vPosition, 1.0);  // Apply all transformations
}

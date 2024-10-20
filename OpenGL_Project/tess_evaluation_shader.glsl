#version 430 core

layout(triangles, equal_spacing, ccw) in;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

void main() {
    // Interpolate positions between the three control points
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;
    vec3 position = p0 * gl_TessCoord.x + p1 * gl_TessCoord.y + p2 * gl_TessCoord.z;

    // Apply the model, view, and projection matrices
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
}

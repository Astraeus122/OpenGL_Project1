#version 430 core

layout(vertices = 3) out;

uniform vec3 cameraPos;
uniform mat4 modelMatrix;

void main() {
    // Calculate the distance from the camera to the patch
    vec3 worldPos = (modelMatrix * gl_in[gl_InvocationID].gl_Position).xyz;
    float distance = length(cameraPos - worldPos);

    // Tessellation level based on distance (closer = more tessellation)
    float tessLevel = mix(10.0, 1.0, distance / 100.0);

    gl_TessLevelOuter[0] = tessLevel;
    gl_TessLevelOuter[1] = tessLevel;
    gl_TessLevelOuter[2] = tessLevel;
    gl_TessLevelInner[0] = tessLevel;

    // Pass through the vertex positions
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}

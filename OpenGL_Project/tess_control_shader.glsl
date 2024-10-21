#version 430 core
layout(vertices = 3) out;

uniform vec3 cameraPos;
in vec3 tcPosition[];

void main() {
    // Calculate distance from the camera to the current vertex
    float distance = length(cameraPos - tcPosition[gl_InvocationID]);

    // Adjust tessellation factor based on the distance
    float tessFactor = clamp(20.0 / distance, 1.0, 64.0);  // Adjust values as needed
    gl_TessLevelOuter[0] = tessFactor;
    gl_TessLevelOuter[1] = tessFactor;
    gl_TessLevelOuter[2] = tessFactor;
    gl_TessLevelInner[0] = tessFactor;

    // Pass through positions
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}

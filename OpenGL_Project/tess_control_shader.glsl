#version 430 core

layout(vertices = 3) out;  // For triangles

uniform vec3 cameraPos;
in vec3 vPosition[];  // Vertex positions from vertex shader

void main() {
    // Pass through positions
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    // Calculate the distance from the camera to the midpoint of the triangle
    vec3 midPoint = (vPosition[0] + vPosition[1] + vPosition[2]) / 3.0;
    float distance = length(cameraPos - midPoint);

    // Set tessellation level based on distance (higher tessellation closer to the camera)
    float tessellationFactor = clamp(50.0 / distance, 1.0, 10.0);  // Adjust LOD scaling here

    gl_TessLevelOuter[0] = tessellationFactor;
    gl_TessLevelOuter[1] = tessellationFactor;
    gl_TessLevelOuter[2] = tessellationFactor;
    gl_TessLevelInner[0] = tessellationFactor;
}

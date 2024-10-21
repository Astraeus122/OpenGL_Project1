#version 430 core

layout(triangles, equal_spacing, ccw) in;

void main() {
    // Interpolate vertex positions
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    // Calculate the position based on barycentric coordinates
    vec3 pos = p0 * gl_TessCoord.x + p1 * gl_TessCoord.y + p2 * gl_TessCoord.z;

    // Output position
    gl_Position = vec4(pos, 1.0);
}

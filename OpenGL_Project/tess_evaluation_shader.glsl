#version 430 core

layout(triangles, equal_spacing, ccw) in;

uniform sampler2D heightmap;
uniform float maxHeight;

void main() {
    // Interpolate vertex positions
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;

    // Calculate the barycentric position of the tessellated point
    vec3 pos = p0 * gl_TessCoord.x + p1 * gl_TessCoord.y + p2 * gl_TessCoord.z;

    // Sample height from the heightmap
    float height = texture(heightmap, pos.xz).r * maxHeight;

    // Displace the tessellated position
    pos.y += height;

    // Output the final position
    gl_Position = vec4(pos, 1.0);
}

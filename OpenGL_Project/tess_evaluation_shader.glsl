#version 430 core
layout(triangles, equal_spacing, cw) in;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform sampler2D heightmap;

in vec2 teTexCoord[];
out vec2 fragTexCoord;

void main() {
    vec3 position = mix(mix(gl_in[0].gl_Position.xyz, gl_in[1].gl_Position.xyz, gl_TessCoord.x),
                        gl_in[2].gl_Position.xyz, gl_TessCoord.y);

    // Displace vertices based on heightmap
    float heightValue = texture(heightmap, teTexCoord[0]).r;
    position.y += heightValue * 20.0;  // Adjust height scaling

    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);
    fragTexCoord = teTexCoord[0];
}

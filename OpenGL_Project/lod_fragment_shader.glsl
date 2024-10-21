#version 430 core
out vec4 fragColor;

in vec2 fragTexCoord;
uniform sampler2D terrainTexture;

void main() {
    fragColor = texture(terrainTexture, fragTexCoord);
}

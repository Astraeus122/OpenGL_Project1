#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec4 color = texture(screenTexture, TexCoords);
    FragColor = vec4(vec3(1.0 - color.rgb), color.a); // Invert colors
}

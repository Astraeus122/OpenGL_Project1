#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;

void main()
{
    vec4 color = texture(screenTexture, TexCoords);
    float grey = dot(color.rgb, vec3(0.299, 0.587, 0.114)); // Standard luminance calculation
    FragColor = vec4(vec3(grey), color.a); // Convert to greyscale
}

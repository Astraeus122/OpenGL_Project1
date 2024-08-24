#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

void main()
{
    vec4 color = texture(screenTexture, TexCoords);
    float rainEffect = abs(sin(TexCoords.y * 100.0 + time * 10.0)) * 0.1; // Simple sine wave rain effect
    FragColor = vec4(color.rgb - rainEffect, color.a); // Apply rain effect
}

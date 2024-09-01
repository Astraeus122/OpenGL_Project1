#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;

void main() {
    float kernelX[9] = float[](
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    );

    float kernelY[9] = float[](
        -1, -2, -1,
         0,  0,  0,
         1,  2,  1
    );

    vec2 tex_offset = 1.0 / textureSize(screenTexture, 0); // gets size of single texel
    vec3 resultX = vec3(0.0);
    vec3 resultY = vec3(0.0);

    for(int i = -1; i <= 1; i++) {
        for(int j = -1; j <= 1; j++) {
            vec2 offset = vec2(tex_offset.x * j, tex_offset.y * i);
            vec3 sample = texture(screenTexture, TexCoords + offset).rgb;

            resultX += sample * kernelX[(i+1)*3 + (j+1)];
            resultY += sample * kernelY[(i+1)*3 + (j+1)];
        }
    }

    float edge = length(resultX) + length(resultY);
    FragColor = vec4(vec3(edge), 1.0);
}

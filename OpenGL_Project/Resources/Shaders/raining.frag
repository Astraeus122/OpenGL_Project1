#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform float time;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float noise(vec2 co)
{
    vec2 i = floor(co);
    vec2 f = fract(co);
    vec2 u = f*f*(3.0-2.0*f);
    return mix(mix(rand(i), rand(i + vec2(1.0, 0.0)), u.x),
               mix(rand(i + vec2(0.0, 1.0)), rand(i + vec2(1.0, 1.0)), u.x), u.y);
}

void main()
{
    vec2 uv = TexCoords;
    vec2 noiseCoord = uv * vec2(80.0, 40.0) + vec2(0.0, time * 20.0); 

    float drop = noise(noiseCoord);
    drop = smoothstep(0.85, 1.0, drop);

    vec2 dropCenter = fract(uv * vec2(80.0, 40.0));
    float dropShape = smoothstep(0.0, 0.1, dropCenter.x) * smoothstep(0.2, 0.1, dropCenter.x);
    drop *= dropShape;

    drop *= smoothstep(0.0, 0.05, dropCenter.y) * smoothstep(1.0, 0.95, dropCenter.y);

    uv.y += drop * 0.3; // Adjust streak length for faster fall

    vec3 color = texture(screenTexture, uv).rgb;
    color = mix(color, vec3(0.2, 0.4, 1.0), drop * 0.5);

    FragColor = vec4(color, 1.0);
}

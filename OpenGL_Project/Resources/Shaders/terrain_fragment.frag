#version 330 core

in vec3 FragPos; 
in vec3 Normal; 
in vec2 TexCoords; // Texture coordinates from vertex shader

uniform vec3 lightDir; // Direction of the light
uniform vec3 lightColor; // Color of the light
uniform vec3 viewPos; // Position of the viewer/camera

uniform sampler2D grassTexture;
uniform sampler2D dirtTexture;
uniform sampler2D rockTexture;
uniform sampler2D snowTexture;

out vec4 FragColor;

void main() 
{
    // Sample the textures
    vec4 grass = texture(grassTexture, TexCoords);
    vec4 dirt = texture(dirtTexture, TexCoords);
    vec4 rock = texture(rockTexture, TexCoords);
    vec4 snow = texture(snowTexture, TexCoords);
    
    // Blend textures based on height (using TexCoords.y as the height)
    vec4 terrainColor = grass; // Default to grass
    if (TexCoords.y < 0.25)
    {
        terrainColor = mix(grass, dirt, TexCoords.y / 0.25);
    } 
    else if (TexCoords.y < 0.5) 
    {
        terrainColor = mix(dirt, rock, (TexCoords.y - 0.25) / 0.25);
    } 
    else if (TexCoords.y < 0.75) 
    {
        terrainColor = mix(rock, snow, (TexCoords.y - 0.5) / 0.25);
    } 
    else 
    {
        terrainColor = snow;
    }

    // Lighting calculations
    vec3 norm = normalize(Normal);
    float diff = max(dot(norm, -lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    // Apply lighting to terrain color
    vec3 finalColor = diffuse * terrainColor.rgb;
    
    FragColor = vec4(finalColor, 1.0);
}

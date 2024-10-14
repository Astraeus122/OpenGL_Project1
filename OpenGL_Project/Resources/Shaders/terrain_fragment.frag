// terrain_fragment_shader.glsl
#version 330 core

in vec3 FragPos; 
in vec3 Normal; 
in float Height; // Pass height as a float from the vertex shader

uniform mat4 lightSpaceMatrix1;
uniform sampler2D shadowMap1;
uniform mat4 lightSpaceMatrix2;
uniform sampler2D shadowMap2;

uniform vec3 lightDir1; // Direction of light 1
uniform vec3 lightDir2; // Direction of light 2
uniform vec3 lightColor1; // Color of light 1
uniform vec3 lightColor2; // Color of light 2

uniform vec3 viewPos; // Position of the camera/viewer

uniform bool isTerrain; 

// Function to calculate the shadow factor
float calculateShadow(vec4 fragPosLightSpace, sampler2D shadowMap)
{
    // Perform perspective divide to get normalized device coordinates
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range to match texture coordinates
    projCoords = projCoords * 0.5 + 0.5;

    // Get closest depth value from the shadow map
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // Get current fragment depth
    float currentDepth = projCoords.z;

    // Bias to prevent shadow acne
    float bias = 0.01;

    // Percentage Closer Filtering (PCF)
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -3; x <= 3; ++x)
    {
        for(int y = -3; y <= 3; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    shadow /= 49.0; // Averaging over 7x7 samples


    // Keep the shadow at 0.0 when outside the far_plane region of the light's orthographic projection
    if(projCoords.z > 1.0)
        shadow = 0.0;

    return shadow;
}

void main()
{
    vec3 terrainColor = vec3(1.0); // White

    // Normalize normals
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Light 1 calculations
    vec3 lightDir1Norm = normalize(-lightDir1);
    float diff1 = max(dot(norm, lightDir1Norm), 0.0);
    vec3 diffuse1 = diff1 * lightColor1;

    // Specular for light 1
    vec3 reflectDir1 = reflect(-lightDir1Norm, norm);
    float spec1 = pow(max(dot(viewDir, reflectDir1), 0.0), 64.0);
    vec3 specular1 = vec3(0.5) * spec1;

    // Shadow for light 1
    vec4 fragPosLightSpace1 = lightSpaceMatrix1 * vec4(FragPos, 1.0);
    float shadow1 = calculateShadow(fragPosLightSpace1, shadowMap1);

    // Lighting contribution from light 1
    vec3 lighting1 = (diffuse1 + specular1) * (1.0 - shadow1);

    // Light 2 calculations
    vec3 lightDir2Norm = normalize(-lightDir2);
    float diff2 = max(dot(norm, lightDir2Norm), 0.0);
    vec3 diffuse2 = diff2 * lightColor2;

    // Specular for light 2
    vec3 reflectDir2 = reflect(-lightDir2Norm, norm);
    float spec2 = pow(max(dot(viewDir, reflectDir2), 0.0), 64.0);
    vec3 specular2 = vec3(0.5) * spec2;

    // Shadow for light 2
    vec4 fragPosLightSpace2 = lightSpaceMatrix2 * vec4(FragPos, 1.0);
    float shadow2 = calculateShadow(fragPosLightSpace2, shadowMap2);

    // Lighting contribution from light 2
    vec3 lighting2 = (diffuse2 + specular2) * (1.0 - shadow2);

    // Total lighting
    vec3 lighting = lighting1 + lighting2;

    // Final color with lighting applied to gray terrain
    vec3 finalColor = lighting * terrainColor;
    FragColor = vec4(finalColor, 1.0);
}

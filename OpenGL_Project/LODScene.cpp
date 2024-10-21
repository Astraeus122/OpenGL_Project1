#include "LODScene.h"
#include <iostream>
#include <vector>
#include "Dependencies/stb_image.h"
#include "Dependencies/glm/gtc/type_ptr.hpp"

// Texture loading utility function (if not using a separate Texture class)
GLuint LoadTexture(const char* path, bool gammaCorrection = false)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    // Flip textures on load
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Texture Parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cerr << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

LODScene::LODScene(ShaderLoader& shaderLoader, Camera& camera)
    : shaderLoader(shaderLoader), camera(camera),
    triangleProgram(0), quadProgram(0), terrainProgram(0),
    triangleVAO(0), triangleVBO(0),
    quadVAO(0), quadVBO(0), quadEBO(0),
    terrainVAO(0), terrainVBO(0), terrainEBO(0),
    triangleTexture(0), quadTexture(0),
    terrainHeightmap(0), terrainTexture(0),
    terrainResolution(32) // Default resolution
{
}

LODScene::~LODScene()
{
    // Cleanup Shader Programs
    if (triangleProgram)
        glDeleteProgram(triangleProgram);
    if (quadProgram)
        glDeleteProgram(quadProgram);
    if (terrainProgram)
        glDeleteProgram(terrainProgram);

    // Cleanup VAOs and VBOs
    if (triangleVAO)
        glDeleteVertexArrays(1, &triangleVAO);
    if (triangleVBO)
        glDeleteBuffers(1, &triangleVBO);
    if (quadVAO)
        glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO)
        glDeleteBuffers(1, &quadVBO);
    if (quadEBO)
        glDeleteBuffers(1, &quadEBO);
    if (terrainVAO)
        glDeleteVertexArrays(1, &terrainVAO);
    if (terrainVBO)
        glDeleteBuffers(1, &terrainVBO);
    if (terrainEBO)
        glDeleteBuffers(1, &terrainEBO);

    // Cleanup Textures
    if (triangleTexture)
        glDeleteTextures(1, &triangleTexture);
    if (quadTexture)
        glDeleteTextures(1, &quadTexture);
    if (terrainHeightmap)
        glDeleteTextures(1, &terrainHeightmap);
    if (terrainTexture)
        glDeleteTextures(1, &terrainTexture);
}

void LODScene::initialize()
{
    // Load and compile shader programs
    // Triangle Shader Program
    triangleProgram = shaderLoader.CreateProgram("triangle_vertex.txt", "triangle_fragment.txt");
    // Attach Tessellation Control and Evaluation Shaders
    GLuint tessControlShader = shaderLoader.CreateShader(GL_TESS_CONTROL_SHADER, "triangle_tess_control.txt");
    GLuint tessEvalShader = shaderLoader.CreateShader(GL_TESS_EVALUATION_SHADER, "triangle_tess_eval.txt");
    glAttachShader(triangleProgram, tessControlShader);
    glAttachShader(triangleProgram, tessEvalShader);
    glLinkProgram(triangleProgram);

    // Check for linking errors
    GLint success;
    glGetProgramiv(triangleProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024];
        glGetProgramInfoLog(triangleProgram, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::TRIANGLE::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(tessControlShader);
    glDeleteShader(tessEvalShader);

    // Quad Shader Program
    quadProgram = shaderLoader.CreateProgram("quad_vertex.txt", "quad_fragment.txt");
    GLuint quadTessControlShader = shaderLoader.CreateShader(GL_TESS_CONTROL_SHADER, "quad_tess_control.txt");
    GLuint quadTessEvalShader = shaderLoader.CreateShader(GL_TESS_EVALUATION_SHADER, "quad_tess_eval.txt");
    glAttachShader(quadProgram, quadTessControlShader);
    glAttachShader(quadProgram, quadTessEvalShader);
    glLinkProgram(quadProgram);

    glGetProgramiv(quadProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024];
        glGetProgramInfoLog(quadProgram, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::QUAD::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(quadTessControlShader);
    glDeleteShader(quadTessEvalShader);

    // Terrain Shader Program
    terrainProgram = shaderLoader.CreateProgram("terrain_vertex.txt", "terrain_fragment.txt");
    GLuint terrainTessControlShader = shaderLoader.CreateShader(GL_TESS_CONTROL_SHADER, "terrain_tess_control.txt");
    GLuint terrainTessEvalShader = shaderLoader.CreateShader(GL_TESS_EVALUATION_SHADER, "terrain_tess_eval.txt");
    glAttachShader(terrainProgram, terrainTessControlShader);
    glAttachShader(terrainProgram, terrainTessEvalShader);
    glLinkProgram(terrainProgram);

    glGetProgramiv(terrainProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024];
        glGetProgramInfoLog(terrainProgram, 1024, NULL, infoLog);
        std::cerr << "ERROR::SHADER::TERRAIN::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(terrainTessControlShader);
    glDeleteShader(terrainTessEvalShader);

    // Setup Geometry
    setupTriangle();
    setupQuad();
    setupTerrain();

    // Load Textures
    loadTextures();
}

void LODScene::setupTriangle()
{
    // Define a single triangle with positions and texture coordinates
    float triangleVertices[] = {
        // positions         // texcoords
        0.0f,  0.0f, 0.0f,    0.5f, 1.0f,  // Vertex 1
        50.0f, 0.0f, 0.0f,    1.0f, 0.0f,  // Vertex 2
        0.0f, 50.0f, 0.0f,    0.0f, 0.0f   // Vertex 3
    };

    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);

    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0); // layout(location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Texture Coord attribute
    glEnableVertexAttribArray(1); // layout(location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void LODScene::setupQuad()
{
    // Define a single quad using four vertices with positions and texture coordinates
    float quadVertices[] = {
        // positions            // texcoords
        -50.0f, 0.0f, -50.0f,   0.0f, 0.0f,
         50.0f, 0.0f, -50.0f,   1.0f, 0.0f,
         50.0f, 0.0f,  50.0f,   1.0f, 1.0f,
        -50.0f, 0.0f,  50.0f,   0.0f, 1.0f
    };
    unsigned int quadIndices[] = {
        0, 1, 2, 3
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0); // layout(location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Texture Coord attribute
    glEnableVertexAttribArray(1); // layout(location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0);
}

void LODScene::setupTerrain()
{
    // Generate terrain grid
    calculateTerrainGeometry();

    glGenVertexArrays(1, &terrainVAO);
    glGenBuffers(1, &terrainVBO);
    glGenBuffers(1, &terrainEBO);

    glBindVertexArray(terrainVAO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, terrainVertices.size() * sizeof(float), terrainVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, terrainIndices.size() * sizeof(unsigned int), terrainIndices.data(), GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0); // layout(location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void LODScene::calculateTerrainGeometry()
{
    // Create a grid for the terrain
    float size = 100.0f; // Total size of the terrain
    float halfSize = size / 2.0f;
    float step = size / terrainResolution;

    for (int z = 0; z <= terrainResolution; ++z)
    {
        for (int x = 0; x <= terrainResolution; ++x)
        {
            float xpos = -halfSize + x * step;
            float zpos = -halfSize + z * step;
            float ypos = 0.0f; // Will be set in TES using heightmap
            terrainVertices.push_back(xpos);
            terrainVertices.push_back(ypos);
            terrainVertices.push_back(zpos);
        }
    }

    // Generate indices for quads
    for (int z = 0; z < terrainResolution; ++z)
    {
        for (int x = 0; x < terrainResolution; ++x)
        {
            int topLeft = z * (terrainResolution + 1) + x;
            int topRight = topLeft + 1;
            int bottomLeft = (z + 1) * (terrainResolution + 1) + x;
            int bottomRight = bottomLeft + 1;

            // Define quad as a patch
            terrainIndices.push_back(topLeft);
            terrainIndices.push_back(topRight);
            terrainIndices.push_back(bottomRight);
            terrainIndices.push_back(bottomLeft);
        }
    }
}

void LODScene::loadTextures()
{
    // Load Triangle Texture
    triangleTexture = LoadTexture("Resources/Textures/texture1.jpg");
    if (triangleTexture == 0)
        std::cerr << "Failed to load triangle texture." << std::endl;

    // Load Quad Texture
    quadTexture = LoadTexture("Resources/Textures/texture2.jpg");
    if (quadTexture == 0)
        std::cerr << "Failed to load quad texture." << std::endl;

    // Load Terrain Heightmap (Grayscale Image)
    terrainHeightmap = LoadTexture("Resources/Heightmap0.jpg");
    if (terrainHeightmap == 0)
        std::cerr << "Failed to load terrain heightmap." << std::endl;

    // Load Terrain Texture
    terrainTexture = LoadTexture("Resources/Textures/texture3.jpg");
    if (terrainTexture == 0)
        std::cerr << "Failed to load terrain texture." << std::endl;
}

void LODScene::render()
{
    // Retrieve camera parameters
    glm::vec3 camPos = camera.getPosition();
    glm::mat4 projection = camera.GetProjectionMatrix();
    glm::mat4 view = camera.GetViewMatrix();

    // Render Tessellated Triangle
    glUseProgram(triangleProgram);

    // Define Model Matrix for Triangle
    glm::mat4 triangleModel = glm::mat4(1.0f); // Identity matrix
    triangleModel = glm::translate(triangleModel, glm::vec3(-15.0f, 10.0f, -30.0f)); // Move left, elevate, and move forward
    triangleModel = glm::scale(triangleModel, glm::vec3(0.5f)); // Optional: Scale down

    // Set Uniforms
    GLint projLoc = glGetUniformLocation(triangleProgram, "projection");
    GLint viewLoc = glGetUniformLocation(triangleProgram, "view");
    GLint modelLoc = glGetUniformLocation(triangleProgram, "model");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(triangleModel));

    // Bind Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, triangleTexture);
    glUniform1i(glGetUniformLocation(triangleProgram, "texture1"), 0);

    // Draw Triangle as Patch
    glBindVertexArray(triangleVAO);
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    glDrawArrays(GL_PATCHES, 0, 3);
    glBindVertexArray(0);

    // Render Tessellated Quad with LOD
    glUseProgram(quadProgram);

    // Define Model Matrix for Quad
    glm::mat4 quadModel = glm::mat4(1.0f); // Identity matrix
    quadModel = glm::translate(quadModel, glm::vec3(15.0f, 15.0f, -50.0f)); // Move right, elevate, and move further forward
    quadModel = glm::scale(quadModel, glm::vec3(0.75f)); // Optional: Scale down

    // Set Uniforms
    projLoc = glGetUniformLocation(quadProgram, "projection");
    viewLoc = glGetUniformLocation(quadProgram, "view");
    GLint quadModelLoc = glGetUniformLocation(quadProgram, "model");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(quadModelLoc, 1, GL_FALSE, glm::value_ptr(quadModel));

    // Calculate distance from camera to quad
    glm::vec3 quadPos = glm::vec3(15.0f, 15.0f, -50.0f);
    float distance = glm::distance(camPos, quadPos);

    // Define distance thresholds for LOD
    float maxDistance = 100.0f; // Beyond this, lowest detail
    float minDistance = 10.0f;  // Closer than this, highest detail

    // Calculate tessellation levels based on distance
    float t = glm::clamp(1.0f - (distance - minDistance) / (maxDistance - minDistance), 0.0f, 1.0f);
    float innerLevel = glm::mix(1.0f, 7.0f, t); // Inner levels from 1 to 7
    float outerLevel = glm::mix(1.0f, 5.0f, t); // Outer levels from 1 to 5

    glUniform1f(glGetUniformLocation(quadProgram, "innerLevel"), innerLevel);
    glUniform1f(glGetUniformLocation(quadProgram, "outerLevel"), outerLevel);

    // Bind Quad Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, quadTexture);
    glUniform1i(glGetUniformLocation(quadProgram, "texture1"), 0);

    // Draw Quad as Patch
    glBindVertexArray(quadVAO);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawElements(GL_PATCHES, 4, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Render Tessellated Terrain
    glUseProgram(terrainProgram);

    // Define Model Matrix for Terrain
    glm::mat4 terrainModel = glm::mat4(1.0f); // Identity matrix

    // Set Uniforms
    projLoc = glGetUniformLocation(terrainProgram, "projection");
    viewLoc = glGetUniformLocation(terrainProgram, "view");
    GLint terrainModelLoc = glGetUniformLocation(terrainProgram, "model");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(terrainModelLoc, 1, GL_FALSE, glm::value_ptr(terrainModel));

    // Bind Heightmap Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, terrainHeightmap);
    glUniform1i(glGetUniformLocation(terrainProgram, "heightmap"), 0);

    // Bind Terrain Texture
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, terrainTexture);
    glUniform1i(glGetUniformLocation(terrainProgram, "terrainTexture"), 1);

    // Draw Terrain as Patch
    glBindVertexArray(terrainVAO);
    glPatchParameteri(GL_PATCH_VERTICES, 4);
    glDrawElements(GL_PATCHES, static_cast<GLsizei>(terrainIndices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


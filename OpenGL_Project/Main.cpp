#define TINYOBJLOADER_IMPLEMENTATION

#include <glew.h>
#include <glfw3.h>
#include <iostream>
#include <vector>
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "ShaderLoader.h"
#include "Camera.h"
#include "ModelLoader.h"
#include "InstancedRenderer.h"
#include "InputHandler.h"
#include "Skybox.h"
#include "Texture.h"
#include "LightManager.h"
#include "StencilTestScene.h"
//#include "PostProcessingScene.h"
#include "Dependencies/stb_image.h"
#include "PerlinNoiseScene.h"


const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

enum Scene {
    SCENE_STENCIL_TEST = 1,
    SCENE_TERRAIN_RENDERING,
    SCENE_PERLIN_NOISE,
    SCENE_POST_PROCESSING
};

GLuint framebuffer, textureColorbuffer, rbo;
GLuint quadVAO = 0;
GLuint quadVBO;

int currentEffect = 0; // For cycling through effects
bool tabKeyPressed = false;

GLFWwindow* initWindow();
void processInput(GLFWwindow* window, Camera& camera, InputHandler& inputHandler, LightManager& lightManager, float deltaTime);
void processSceneInput(GLFWwindow* window, Scene& currentScene);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

Camera* camera;
Scene currentScene = SCENE_STENCIL_TEST; // Start with the stencil test scene by default

int main() {
    GLFWwindow* window = initWindow();
    if (!window) {
        return -1;
    }

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);

    ShaderLoader shaderLoader;
    GLuint shaderProgram = shaderLoader.CreateProgram(
        "Resources/Shaders/vertex_shader.vert",
        "Resources/Shaders/fragment_shader.frag"
    );

    GLuint perlinShader = shaderLoader.CreateProgram(
        "perlin_vertex_shader.txt",
        "perlin_fragment_shader.txt"
    );

    // Initialize framebuffer
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a color attachment texture
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // Create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Adjust camera position and clipping planes
    Camera cam(window, glm::vec3(0.0f, 50.0f, 100.0f), 5.0f);
    camera = &cam;
    InputHandler inputHandler(window);

    // Initialize LightManager
    LightManager lightManager;
    lightManager.initialize();

    // Load the turret model
    ModelLoader mineModelLoader("Resources/Models/SciFiSpace/SM_Prop_Mine_01.obj");
    mineModelLoader.loadModel();

    // Initialize instance renderer for the turret model
    InstancedRenderer mineRenderer(
        mineModelLoader.getPositions(),
        mineModelLoader.getTexCoords(),
        mineModelLoader.getNormals(),
        "Resources/Textures/PolygonSciFiSpace_Texture_01_A.png",
        10 // Number of instances
    );
    mineRenderer.initialize();

    const std::vector<std::string> skyboxFaces = {
        "Resources/Skybox/Right.png",
        "Resources/Skybox/Left.png",
        "Resources/Skybox/Top.png",
        "Resources/Skybox/Bottom.png",
        "Resources/Skybox/Back.png",
        "Resources/Skybox/Front.png"
    };

    // Skybox setup
    Skybox skybox(skyboxFaces);

    // Initialize scenes
    StencilTestScene stencilTestScene(shaderLoader, cam, skybox, mineRenderer);
    PerlinNoiseScene perlinNoiseScene(shaderLoader, cam);
    perlinNoiseScene.initialize();

    // Initialize PerlinNoise
    GLuint noiseTexture;
    bool scene3Initialized = false;

    glEnable(GL_DEPTH_TEST);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processSceneInput(window, currentScene);  // Handle scene switching input
        processInput(window, cam, inputHandler, lightManager, deltaTime);  // Handle camera and light input

        // Ensure the camera updates before passing light data
        cam.update(deltaTime);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Common rendering: Skybox
        glm::mat4 view = glm::mat4(glm::mat3(cam.getViewMatrix()));
        skybox.render(cam.getProjectionMatrix() * view);

        switch (currentScene) 
        {
        case SCENE_STENCIL_TEST:
        {
            stencilTestScene.render();
            break;
        }
        case SCENE_PERLIN_NOISE:
        {
            perlinNoiseScene.render();
            break;
        }
        case SCENE_POST_PROCESSING: 
        {
            //postProcessingScene.render(currentEffect, quadVAO);
            break;
        }

                                  // Add your other cases here as needed

        default:
            break;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

GLFWwindow* initWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    return window;
}

void processSceneInput(GLFWwindow* window, Scene& currentScene) {
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        currentScene = SCENE_STENCIL_TEST;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        currentScene = SCENE_TERRAIN_RENDERING;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        currentScene = SCENE_PERLIN_NOISE;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        currentScene = SCENE_POST_PROCESSING;
    }
}

void processInput(GLFWwindow* window, Camera& camera, InputHandler& inputHandler, LightManager& lightManager, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    camera.processInput(deltaTime);
    inputHandler.processInput(deltaTime);

    static bool pointLightTogglePressed = false;
    static bool directionalLightTogglePressed = false;
    static bool spotLightTogglePressed = false;

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        if (!pointLightTogglePressed) {
            lightManager.pointLightsOn = !lightManager.pointLightsOn;
            pointLightTogglePressed = true;
        }
    }
    else {
        pointLightTogglePressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        if (!directionalLightTogglePressed) {
            lightManager.directionalLightOn = !lightManager.directionalLightOn;
            directionalLightTogglePressed = true;
        }
    }
    else {
        directionalLightTogglePressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        if (!spotLightTogglePressed) {
            lightManager.spotLightOn = !lightManager.spotLightOn;
            spotLightTogglePressed = true;
        }
    }
    else {
        spotLightTogglePressed = false;
    }

    // Handle post-processing effect cycling
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabKeyPressed) {
        currentEffect = (currentEffect + 1) % 5; // Assuming 5 effects (0 to 4)
        tabKeyPressed = true;
    }
    else if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE) {
        tabKeyPressed = false;
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = 400.0f;
    static float lastY = 300.0f;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera->processMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->processMouseScroll(yoffset);
}

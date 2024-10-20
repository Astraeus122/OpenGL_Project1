#include <iostream>
#include <vector>
#include <cstdlib> // For rand()
#include <ctime>   // For seeding rand()
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/ext/scalar_common.hpp" // For scalar lerp
#include "Dependencies/glm/ext/vector_common.hpp"
#include "Dependencies/glew/glew.h"
#include "Dependencies/glfw/glfw3.h"
#include "ShaderLoader.h"
#include "Camera.h"
#include "ModelLoader.h"
#include "InstancedRenderer.h"
#include "InputHandler.h"
#include "Skybox.h"
#include "Texture.h"
#include "LightManager.h"
#include "PostProcessingScene.h"
#include "PerlinNoiseScene.h"
#include "ShadowScene.h" 
#include "DeferredScene.h"
#include "Plane.h"
#include "ParticleSystem.h"
#include "LODScene.h"

// Screen dimensions
const unsigned int WIDTH = 1920;
const unsigned int HEIGHT = 1080;

// Enum for scene management
enum Scene
{
    SCENE_SHADOW,
    SCENE_DEFERRED_RENDERING,
    SCENE_COMPUTE_SHADER,
    SCENE_LOD
};

// Global variables for framebuffer (if needed)
GLuint framebuffer, textureColorbuffer, rbo;
GLuint quadVAO = 0;
GLuint quadVBO;

// Variables for input handling
int currentEffect = 0; // For cycling through effects
bool tabKeyPressed = false;

// Function declarations
GLFWwindow* initWindow();
void processInput(GLFWwindow* window, Camera& camera, InputHandler& inputHandler, LightManager& lightManager, float deltaTime, ShadowScene& shadowScene, Scene currentScene);
void processSceneInput(GLFWwindow* window, Scene& currentScene);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Camera pointer
Camera* camera;
Scene currentScene = SCENE_SHADOW; // Start with Shadow scene by default

// Initialize Quad VAO for post-processing or screen-space effects
void initQuadVAO() {
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    // TexCoord attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);
}

// Initialize Framebuffers (if needed for other scenes)
void initFramebuffers() {
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a color attachment texture
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // Create a renderbuffer object for depth and stencil attachment
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // Check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    }
    else {
        std::cout << "Framebuffer is complete and ready for rendering!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


// Global ParticleSystem pointer
ParticleSystem* particleSystem = nullptr;

int main()
{
    // Seed random number generator
    std::srand(static_cast<unsigned int>(std::time(0)));

    // Initialize GLFW window
    GLFWwindow* window = initWindow();
    if (!window) {
        return -1;
    }

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        std::cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // Set viewport
    glViewport(0, 0, WIDTH, HEIGHT);

    // Initialize ShaderLoader
    ShaderLoader shaderLoader;

    // Initialize Framebuffers (if needed for other scenes)
    initFramebuffers();

    // Initialize Camera
    Camera cam(window, glm::vec3(0.0f, 50.0f, 100.0f), 5.0f);
    camera = &cam;

    // Initialize InputHandler
    InputHandler inputHandler(window);

    // Initialize LightManager
    LightManager lightManager;
    lightManager.initialize();

    // Initialize ParticleSystem
    particleSystem = new ParticleSystem("particle_compute.txt", "particle_vertex.txt", "particle_fragment.txt");
    particleSystem->init();

    // Load Models
    ModelLoader mineModelLoader("Resources/Models/SciFiSpace/SM_Prop_Mine_01.obj");
    mineModelLoader.loadModel();

    ModelLoader alienModelLoader("Resources/Models/SciFiWorlds/SM_Env_Artifact_AlienRuin_03.obj");
    alienModelLoader.loadModel();

    ModelLoader cannonModelLoader("Resources/Models/SciFiWorlds/SM_Bld_Planetary_Cannon_01.obj");
    cannonModelLoader.loadModel();

    // Initialize Instanced Renderers for models
    InstancedRenderer mineRenderer(
        "Resources/Models/SciFiSpace/SM_Prop_Mine_01.obj",
        "Resources/Textures/PolygonSciFiSpace_Texture_01_A.png",
        10
    );
    mineRenderer.initialize();

    LODScene lodScene(shaderLoader, cam);
    lodScene.initialize();  // Initialize LOD Scene

    // Uncomment and properly initialize other renderers if needed
    /*
    InstancedRenderer alienRenderer(
        alienModelLoader.getPositions(),
        alienModelLoader.getTexCoords(),
        alienModelLoader.getNormals(),
        "Resources/Textures/PolygonSciFiSpace_Texture_01_A.png",
        10
    );
    alienRenderer.initialize();

    InstancedRenderer canonRenderer(
        cannonModelLoader.getPositions(),
        cannonModelLoader.getTexCoords(),
        cannonModelLoader.getNormals(),
        "Resources/Textures/PolygonSciFiSpace_Texture_01_A.png",
        10
    );
    canonRenderer.initialize();
    */

    // Define skybox faces
    const std::vector<std::string> skyboxFaces = {
        "Resources/Skybox/Right.png",
        "Resources/Skybox/Left.png",
        "Resources/Skybox/Top.png",
        "Resources/Skybox/Bottom.png",
        "Resources/Skybox/Back.png",
        "Resources/Skybox/Front.png"
    };

    // Initialize Skybox
    Skybox skybox(skyboxFaces);

    // Initialize other scenes
    PerlinNoiseScene perlinNoiseScene(shaderLoader, cam);
    perlinNoiseScene.initialize();

    // Initialize Shadow Scene
    ShadowScene shadowScene(shaderLoader, cam, skybox, mineRenderer, lightManager);  // Initialize the Shadow Scene
    shadowScene.initialize();  // Setup for Shadow Scene

    // Initialize Deferred Rendering Scene
    DeferredScene deferredRenderingScene(WIDTH, HEIGHT); // Corrected constructor
    deferredRenderingScene.init(shaderLoader); // Initialize with ShaderLoader

    // Initialize Quad VAO for screen-space rendering
    initQuadVAO();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Time variables for frame timing
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    Plane plane;
    plane.loadPlane();
    glm::mat4 viewMatrix;

    viewMatrix = cam.GetViewMatrix();

    // Set input callbacks
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Main rendering loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Handle scene switching input
        processSceneInput(window, currentScene);  // Handle scene switching input

        // Process general input
        processInput(window, cam, inputHandler, lightManager, deltaTime, shadowScene, currentScene);  // Pass ShadowScene and currentScene

        // Update camera
        cam.update(deltaTime);

        // Update particle system (if in compute shader scene)
        if (currentScene == SCENE_COMPUTE_SHADER && particleSystem) {
            particleSystem->update(deltaTime);
        }

        // Clear the screen
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render Skybox
        glm::mat4 view = glm::mat4(glm::mat3(cam.GetViewMatrix()));
        skybox.render(cam.GetProjectionMatrix() * view);

        // Render based on the current scene
        switch (currentScene)
        {
        case SCENE_SHADOW:
            shadowScene.render();  // Render the Shadow Scene
            break;
        case SCENE_DEFERRED_RENDERING:
            // Perform Deferred Rendering Passes
            // 1. Geometry Pass
            deferredRenderingScene.geometryPass({ &mineModelLoader, &alienModelLoader, &cannonModelLoader }, cam.GetViewMatrix(), cam.GetProjectionMatrix());

            // 2. Lighting Pass
            deferredRenderingScene.lightingPass(
                lightManager.getPointLights(),
                lightManager.getDirectionalLight(),
                lightManager.getSpotLight(),
                cam.getPosition()
            );

            // 3. Render Light Boxes
            deferredRenderingScene.renderLights(lightManager.getPointLights(), cam.GetViewMatrix(), cam.GetProjectionMatrix());

            break;
        case SCENE_COMPUTE_SHADER:
            // Clear the screen with a different color if desired
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background for fireworks
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Enable blending for particles
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            // Update particle system
            if (particleSystem) {
                particleSystem->update(deltaTime);
            }

            // Render particles
            if (particleSystem) {
                glm::mat4 viewMatrix = cam.GetViewMatrix();
                glm::mat4 projectionMatrix = cam.GetProjectionMatrix();
                particleSystem->render(viewMatrix, projectionMatrix);
            }

            // Disable blending after rendering particles
            glDisable(GL_BLEND);
            break;
        case SCENE_LOD:
            lodScene.render();  // Add LOD Scene rendering here
            break;
        default:
            break;
        }

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    if (particleSystem) {
        delete particleSystem;
    }

    glfwTerminate();
    return 0;
}

// Function to initialize GLFW window
GLFWwindow* initWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return nullptr;
    }

    // Set OpenGL version (at least 4.3 for compute shaders)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create Window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Demo", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    return window;
}

// Function to handle scene switching input
void processSceneInput(GLFWwindow* window, Scene& currentScene) {
    static bool key1Pressed = false;
    static bool key2Pressed = false;
    static bool key3Pressed = false;
    static bool key4Pressed = false;  // LOD Scene key

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !key1Pressed) {
        currentScene = SCENE_SHADOW;
        key1Pressed = true;
        std::cout << "Switched to Scene 1: Shadows" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE) {
        key1Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !key2Pressed) {
        currentScene = SCENE_DEFERRED_RENDERING;
        key2Pressed = true;
        std::cout << "Switched to Scene 2: Deferred Rendering" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE) {
        key2Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !key3Pressed) {
        currentScene = SCENE_COMPUTE_SHADER;
        key3Pressed = true;
        std::cout << "Switched to Scene 3: Compute Shader (GPU Particles)" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) {
        key3Pressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && !key4Pressed) {
        currentScene = SCENE_LOD;
        key4Pressed = true;
        std::cout << "Switched to Scene 4: LOD Scene" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_RELEASE) {
        key4Pressed = false;
    }
}


// Function to handle general input
void processInput(GLFWwindow* window, Camera& camera, InputHandler& inputHandler, LightManager& lightManager, float deltaTime, ShadowScene& shadowScene, Scene currentScene) {
    // Close window on pressing ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    // Process camera and other inputs
    camera.processInput(deltaTime);
    inputHandler.processInput(deltaTime);

    // Toggle point lights with key '5'
    static bool pointLightTogglePressed = false;
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        if (!pointLightTogglePressed) {
            lightManager.pointLightsOn = !lightManager.pointLightsOn;
            pointLightTogglePressed = true;
            std::cout << "Point Lights " << (lightManager.pointLightsOn ? "On" : "Off") << std::endl;
        }
    }
    else {
        pointLightTogglePressed = false;
    }

    // Toggle directional lights with key '6'
    static bool directionalLightTogglePressed = false;
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        if (!directionalLightTogglePressed) {
            lightManager.directionalLightOn = !lightManager.directionalLightOn;
            directionalLightTogglePressed = true;
            std::cout << "Directional Light " << (lightManager.directionalLightOn ? "On" : "Off") << std::endl;
        }
    }
    else {
        directionalLightTogglePressed = false;
    }

    // Toggle spotlights with key '7'
    static bool spotLightTogglePressed = false;
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        if (!spotLightTogglePressed) {
            lightManager.spotLightOn = !lightManager.spotLightOn;
            spotLightTogglePressed = true;
            std::cout << "Spotlight " << (lightManager.spotLightOn ? "On" : "Off") << std::endl;
        }
    }
    else {
        spotLightTogglePressed = false;
    }

    // Smooth Movement Logic for Movable Model (Only in Shadow Scene)
    if (currentScene == SCENE_SHADOW) {
        ModelLoader& movableModel = shadowScene.getMovableModel();

        // Set a base speed for the model's movement
        float speed = 80.0f * deltaTime;

        // Handle arrow key movement for the model
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            movableModel.translate(camera.front * speed);  // Move forward
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            movableModel.translate(-camera.front * speed);  // Move backward
        }
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            movableModel.translate(-camera.right * speed);  // Move left
        }
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            movableModel.translate(camera.right * speed);  // Move right
        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
            movableModel.translate(camera.up * speed);  // Move up
        }
        if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
            movableModel.translate(-camera.up * speed);  // Move down
        }
    }

    // Trigger Firework with 'F' key (Only in Compute Shader Scene)
    if (currentScene == SCENE_COMPUTE_SHADER && particleSystem) {
        static bool fKeyPressed = false;
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
            if (!fKeyPressed) {
                // Define spawn parameters
                float distance = 30.0f;
                float yOffset = -30.0f; 

                // Trigger multiple fireworks
                for (int i = 0; i < 4; ++i) {
                    // Calculate spawn position with adjusted distance and Y-offset
                    glm::vec3 spawnPos = camera.getPosition() + camera.front * distance
                        + glm::vec3(((rand() % 5) - 2) * 5.0f, yOffset, ((rand() % 5) - 2) * 5.0f); 

                    // Assign random color
                    glm::vec4 color = glm::vec4(static_cast<float>(rand()) / RAND_MAX,
                        static_cast<float>(rand()) / RAND_MAX,
                        static_cast<float>(rand()) / RAND_MAX,
                        1.0f);

                    particleSystem->triggerFirework(spawnPos, color);
                }
                fKeyPressed = true;
                std::cout << "Firework Triggered!" << std::endl;
            }
        }
        else {
            fKeyPressed = false;
        }
    }
}

// Mouse movement callback
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    static float lastX = 400.0f;
    static float lastY = 300.0f;
    static bool firstMouse = true;

    if (firstMouse) {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos); // Reversed since y-coordinates range from bottom to top
    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    camera->processMouseMovement(xoffset, yoffset);
}

// Scroll wheel callback
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    camera->processMouseScroll(static_cast<float>(yoffset));
}

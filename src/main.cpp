#include <iostream>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "Core/Time.h"
#include "Core/InputManager.h"
#include "Core/Renderer.h"
#include "GameStates/GameState.h"

// Window dimensions
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char* WINDOW_TITLE = "Snake Game - OpenGL";

// GLFW error callback
void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW Error: " << description << std::endl;
}

int main() {
    // Initialize GLFW
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    
    // Load OpenGL functions with glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // Initialize game systems
    Time::getInstance()->init();
    InputManager::getInstance()->init(window);
    
    // Create renderer
    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!renderer.init()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }
    
    // Create game state
    GameState gameState(&renderer, InputManager::getInstance());
    gameState.enter();
    
    // Set callbacks (optional)
    gameState.setGameOverCallback([]() {
        std::cout << "Game Over! Press R to restart" << std::endl;
    });
    
    // Main game loop
    while (!glfwWindowShouldClose(window)) {
        // Update time
        Time::getInstance()->update();
        float dt = Time::getInstance()->deltaTime();
        
        // Update input
        InputManager::getInstance()->update();
        
        // Handle restart
        if (InputManager::getInstance()->isKeyJustPressed(GLFW_KEY_R)) {
            gameState.exit();
            gameState.enter();
        }
        
        // Handle quit
        if (InputManager::getInstance()->isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, true);
        }
        
        // Update game logic
        gameState.update(dt);
        
        // Render
        renderer.beginFrame();
        gameState.render();
        renderer.endFrame();
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    gameState.exit();
    
    glfwDestroyWindow(window);
    glfwTerminate();
    
    return 0;
}
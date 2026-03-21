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
    (void)error; // Mark as unused to avoid warning
    std::cerr << "GLFW Error: " << description << std::endl;
}

int main() {
    // 1. INITIALIZE GLFW (Graphics Library Framework)
    // This library handles OS-specific tasks like creating windows, reading keyboard/mouse input, etc.
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // 2. CONFIGURE OPENGL CONTEXT
    // Tell GLFW we want to use OpenGL version 3.3 (Core Profile)
    // "Core Profile" means we won't use backwards-compatible features (old, deprecated functions).
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    // Required for Mac OS X to support modern OpenGL
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // 3. CREATE THE WINDOW
    // Creates an 800x600 window with the title "Snake Game - OpenGL"
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate(); // Clean up GLFW before exiting
        return -1;
    }
    
    // Make this window the current "context", meaning all OpenGL rendering commands will draw to this window.
    glfwMakeContextCurrent(window);
    
    // Enable V-Sync (Vertical Synchronization).
    // SwapInterval(1) tells GPU to wait for the monitor screen to refresh before drawing the next frame, preventing visual "tearing".
    glfwSwapInterval(1); 
    
    // 4. INITIALIZE GLAD
    // GLAD manages function pointers for OpenGL. Because OpenGL implementations vary by graphics card and OS,
    // we need GLAD to find and load all the actual OpenGL rendering functions during runtime.
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // 5. INITIALIZE OUR CUSTOM SYSTEMS
    // Setup our Time system (tracks frame rates) and our InputManager (tracks user key presses)
    Time::getInstance()->init();
    InputManager::getInstance()->init(window);
    
    // Setup our custom Renderer class which sets up shaders and drawing buffers behind the scenes
    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!renderer.init()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }
    
    // 6. INITIALIZE GAME STATE
    // State machine approach. GameState handles the core Snake mechanics.
    GameState gameState(&renderer, InputManager::getInstance());
    gameState.enter(); // Set up the initial state (placing snake, setting score)
    
    // Provide a callback to run whenever the player hits a wall or themselves.
    gameState.setGameOverCallback([]() {
        std::cout << "Game Over! Press R to restart" << std::endl;
    });
    
    // =======================================================
    // 7. THE MAIN GAME LOOP
    // This block of code runs constantly, frame after frame, until the user closes the window.
    // =======================================================
    while (!glfwWindowShouldClose(window)) {
        // A. TIMING
        // Find out how much time has passed since the last loop (delta time "dt")
        Time::getInstance()->update();
        float dt = Time::getInstance()->deltaTime();
        
        // B. INPUT
        // 1) Update custom input manager FIRST to capture the keys from previous frame
        InputManager::getInstance()->update();
        // 2) Collect any newly pressed OS/window events (keyboard, mouse move, window close)
        glfwPollEvents();
        
        // C. GAME CONTROLS (Global Keys)
        // If "ESCAPE" is pressed, signal GLFW to end the main loop next frame.
        if (InputManager::getInstance()->isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, true);
        }
        
        // D. UPDATE LOGIC
        // Feed the time elapsed (dt) into the game so the snake can move at a steady speed 
        gameState.update(dt);
        
        // E. RENDER (DRAW TO SCREEN)
        // 1) Clear the screen to a blank background color
        renderer.beginFrame();
        // 2) Tell the game state to instruct the renderer to draw the snake, food, walls
        gameState.render();
        // 3) Complete any finalizing rendering steps (if any)
        renderer.endFrame();
        
        // Swap buffers: OpenGL draws on a "back buffer" invisibly building the frame.
        // SwapBuffers pushes the fully finished "back buffer" frame onto the screen "front buffer" instantly.
        glfwSwapBuffers(window);
    }
    
    // 8. SHUTDOWN & CLEANUP
    // Once loop exits (game ends), free up resources
    gameState.exit();
    
    glfwDestroyWindow(window); // Destroy the OS window
    glfwTerminate(); // Turn off GLFW completely
    
    return 0;
}
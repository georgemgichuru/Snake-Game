#include <iostream>
#include "glad/glad.h" // MUST be included before GLFW!
#include "GLFW/glfw3.h"
#include "Core/Time.h"
#include "Core/InputManager.h"
#include "Core/Renderer.h"
#include "GameStates/GameState.h"

// Hardcoded Window dimensions and Title
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const char* WINDOW_TITLE = "Snake Game - OpenGL";

// A simple callback function we hand to GLFW. If it fails to open a window, it will print why here.
void glfwErrorCallback(int error, const char* description) {
    (void)error; // Cast to void to prevent compiler "unused variable" warnings
    std::cerr << "GLFW Error: " << description << std::endl;
}

int main() {
    // =======================================================
    // 1. INITIALIZE OS WINDOW MANAGER (GLFW)
    // =======================================================
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) { // Boot up the GLFW library
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    
    // =======================================================
    // 2. CONFIGURE OPENGL SETTINGS
    // =======================================================
    // Tell GLFW we specifically want to use OpenGL Version 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    // "Core Profile" means we explicitly lock out old, deprecated OpenGL functions from the 1990s.
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__ // A C++ Macro! If this code is compiled on a Mac, run the following line.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    
    // =======================================================
    // 3. CREATE THE WINDOW
    // =======================================================
    // Actually ask the Operating System to open the window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate(); // Safely clean up memory before crashing
        return -1;
    }
    
    // Tells OpenGL: "Hey, any draw commands I send from now on should be painted onto THIS specific window."
    glfwMakeContextCurrent(window);
    
    // Enable V-Sync (Vertical Synchronization). 
    // Tells the GPU to wait for your monitor's refresh rate (e.g., 60hz) before swapping the buffers.
    glfwSwapInterval(1); 
    
    // =======================================================
    // 4. INITIALIZE GLAD (GPU Function Detective)
    // =======================================================
    // Hands GLAD the OS-specific function it needs to hunt down the graphics card drivers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // =======================================================
    // 5. INITIALIZE OUR CUSTOM SYSTEMS
    // =======================================================
    // Boot up the singletons
    Time::getInstance()->init();
    InputManager::getInstance()->init(window);
    
    // Boot up our custom OpenGL abstraction wrapper
    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!renderer.init()) {
        std::cerr << "Failed to initialize renderer" << std::endl;
        return -1;
    }
    
    // =======================================================
    // 6. INITIALIZE GAME STATE
    // =======================================================
    // Create the referee that dictates the rules of Snake
    GameState gameState(&renderer, InputManager::getInstance());
    gameState.enter(); // Trigger the boot-up sequence (load sounds, place apple, etc.)
    
    // A C++ Lambda function! This is a tiny, unnamed block of code we give to the GameState.
    // If the snake hits a wall, the GameState will call this.
    gameState.setGameOverCallback([]() {
        std::cout << "Game Over! Press R to restart" << std::endl;
    });
    
    // =======================================================
    // 7. THE MAIN GAME LOOP
    // =======================================================
    // This loops thousands of times a second until you hit the "X" button on the window.
    while (!glfwWindowShouldClose(window)) {
        
        // A. TIMING
        // Calculate exactly how much time passed since the last loop
        Time::getInstance()->update();
        float dt = Time::getInstance()->deltaTime();
        
        // B. INPUT
        // Read keystrokes from the last frame
        InputManager::getInstance()->update();
        
        // Ask the OS if the user moved the mouse, resized the window, or clicked the "X" button
        glfwPollEvents(); 
        
        // C. GLOBAL CONTROLS
        // If "ESCAPE" is pressed, politely ask the window to close, which will break this while loop on the next cycle.
        if (InputManager::getInstance()->isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, true);
        }
        
        // D. LOGIC
        // Advance the game logic (move the snake, check collisions, etc.)
        gameState.update(dt);
        
        // E. RENDERING
        renderer.beginFrame();  // Wipe the slate clean (Clear the screen)
        gameState.render();     // Draw all the sprites to the Back Buffer
        renderer.endFrame();    // Finalize
        
        // F. PRESENTATION
        // Push the perfectly completed Back Buffer onto the screen for the user to see!
        glfwSwapBuffers(window);
    }
    
    // =======================================================
    // 8. SHUTDOWN & CLEANUP
    // =======================================================
    // The player closed the window! Clean up the audio engine and RAM so we don't cause memory leaks.
    gameState.exit();
    
    glfwDestroyWindow(window); // Ask the OS to destroy the window frame
    glfwTerminate(); // Shut down GLFW safely
    
    return 0; // Return 0 means the program closed successfully without errors!
}
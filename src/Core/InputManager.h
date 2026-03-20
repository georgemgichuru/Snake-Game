#pragma once

#include <unordered_map>
#include "GLFW/glfw3.h"

/**
 * InputManager - Handles keyboard and mouse input
 */
class InputManager {
private:
    static InputManager* s_instance;
    
    GLFWwindow* m_window;
    std::unordered_map<int, bool> m_currentKeys;
    std::unordered_map<int, bool> m_previousKeys;
    
    InputManager();
    ~InputManager() = default;
    
    // GLFW callback functions
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    
public:
    // Delete copy constructor and assignment operator
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    
    static InputManager* getInstance();
    
    // Initialize input manager with window
    void init(GLFWwindow* window);
    
    // Update input states (call once per frame)
    void update();
    
    // Check if a key is currently pressed
    bool isKeyPressed(int key) const;
    
    // Check if a key was just pressed this frame
    bool isKeyJustPressed(int key) const;
    
    // Check if a key was just released this frame
    bool isKeyJustReleased(int key) const;
};
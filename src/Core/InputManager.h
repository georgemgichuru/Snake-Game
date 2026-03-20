#pragma once

#include "GLFW/glfw3.h"

/**
 * InputManager Class:
 * Manages tracking which controller/keyboard keys the player presses.
 * It's structured as a "Singleton", meaning there's mathematically only ever
 * ONE InputManager existing in memory anywhere.
 */
class InputManager {
private:
    // Only instance available system-wide
    static InputManager* s_instance;
    GLFWwindow* m_window;
    
    // An array keeping track of all keyboard keys states from the PREVIOUS game frame
    // This allows us to see if a key was "just pressed" instead of "held down"
    bool m_previousKeys[GLFW_KEY_LAST + 1];
    
    // Private constructor (prevents making a second InputManager via `new`)
    InputManager();
    ~InputManager() = default;
    
public:
    // Delete copy constructor so nobody copies our unique singleton reference
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    
    // Retrieves the one universal instance of this InputManager
    static InputManager* getInstance();
    
    // Tell the InputManager which window we are capturing keys from
    void init(GLFWwindow* window);
    
    // Takes a screenshot of current hardware state BEFORE OS polling to use for comparison
    void update();
    
    // Returns `true` if a key is being held down consistently
    bool isKeyPressed(int key) const;
    
    // Returns `true` only for the first frame a key was physically mashed down.
    // Incredibly important for UI and Snake movement so we don't rapid-fire move.
    bool isKeyJustPressed(int key) const;
    
    // Returns `true` exactly on the frame the player let go of the button
    bool isKeyJustReleased(int key) const;
};
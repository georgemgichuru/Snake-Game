#pragma once // Tells the C++ compiler to only include this header file once per compilation unit to avoid duplicate definitions.

#include "GLFW/glfw3.h" // Includes the GLFW library which handles our window creation and keyboard/mouse input.

/**
 * InputManager Class:
 * Manages tracking which controller/keyboard keys the player presses.
 * It's structured as a "Singleton", meaning there's mathematically only ever
 * ONE InputManager existing in memory anywhere.
 */
class InputManager { // Begins the declaration of the InputManager class.
private: // Starts the private section. Everything below this can only be accessed from inside this specific class.
    static InputManager* s_instance; // A static pointer that will hold the one, universally shared instance of this class.
    GLFWwindow* m_window; // A pointer to the specific GLFW window we are capturing input from.
    
    // An array keeping track of all keyboard keys states from the PREVIOUS game frame
    // This allows us to see if a key was "just pressed" instead of "held down"
    bool m_previousKeys[GLFW_KEY_LAST + 1]; // An array of booleans sized to hold every possible GLFW key code.
    
    InputManager(); // The constructor is private! This stops other code from creating a new InputManager using the 'new' keyword.
    ~InputManager() = default; // The destructor is set to the compiler-generated default since we don't have complex manual cleanup here.
    
public: // Starts the public section. Everything below this can be used by the rest of your game code.
    InputManager(const InputManager&) = delete; // "Deletes" the copy constructor. This makes it impossible to accidentally copy our Singleton.
    InputManager& operator=(const InputManager&) = delete; // "Deletes" the assignment operator. Also prevents copying via assignment (e.g., inputA = inputB).
    
    static InputManager* getInstance(); // A public, static function that other files call to get access to the one instance of this class.
    
    void init(GLFWwindow* window); // A function we will call once at startup to tell the manager which window to listen to.
    
    void update(); // A function we will call once every game frame to record the key states.
    
    bool isKeyPressed(int key) const; // Returns true if a key is currently held down. 'const' guarantees this function won't modify class variables.
    
    bool isKeyJustPressed(int key) const; // Returns true ONLY on the exact frame the key is pushed down.
    
    bool isKeyJustReleased(int key) const; // Returns true ONLY on the exact frame the key is let go.
}; // Ends the class declaration.
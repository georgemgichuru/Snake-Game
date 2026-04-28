#include "Core/InputManager.h" // Includes the header file we just looked at so we can implement its functions.
#include <cstring> // Includes the C-string library, which gives us access to memory manipulation functions like 'memset'.

// This is so that we do not get an unresollved external symbol error when we
// try using it without defining it as it is declared in InputManager.h
InputManager *InputManager::s_instance =
    nullptr; // Initializes our static singleton pointer to 'nullptr' (empty) at
             // the very start of the program.

InputManager::InputManager() // Defines the implementation for the constructor.
    : m_window(nullptr) {    // Uses an initialization list to set our window
                             // pointer to null when the object is created.
  memset(
      m_previousKeys, 0,
      sizeof(
          m_previousKeys)); // Overwrites the entire m_previousKeys array with
                            // 0s (false). This clears out any garbage memory.
} // Ends the constructor.

InputManager *InputManager::getInstance() { // Defines the function that gives
                                            // access to our singleton.
  if (!s_instance) { // Checks if the singleton pointer is still null (meaning
                     // it hasn't been created yet).
    s_instance = new InputManager(); // If it hasn't been created, allocate it
                                     // in memory now.
  } // Ends the if statement.
  return s_instance; // Returns the pointer to the globally shared InputManager.
} // Ends the getInstance function.

void InputManager::init(
    GLFWwindow *window) { // Defines the initialization function.
  m_window = window; // Saves the reference to our game's window into our class
                     // variable so we can ask it for key states later.
  memset(m_previousKeys, 0,
         sizeof(m_previousKeys)); // Resets the previous keys array to all false
                                  // (0) just to be safe upon initialization.
} // Ends the init function.

void InputManager::update() { // Defines the update loop, which should be called
                              // at the end of every game frame.
  for (int key = 32; key <= GLFW_KEY_LAST;
       key++) { // Loops through every valid GLFW key code (keys 0-31 are mostly
                // invalid/unmapped in GLFW, spacebar starts at 32).
    m_previousKeys[key] =
        (glfwGetKey(m_window, key) ==
         GLFW_PRESS); // Asks GLFW for the current state of the key, and saves
                      // it into the array for the NEXT frame to use as
                      // "previous".
  } // Ends the for loop.
} // Ends the update function.

bool InputManager::isKeyPressed(
    int key) const { // Defines the function checking if a key is held down.
  if (key < 32 || key > GLFW_KEY_LAST)
    return false; // Safety check: if the key asked for is out of the valid
                  // range, immediately return false.
  return glfwGetKey(m_window, key) ==
         GLFW_PRESS; // Asks GLFW right now if the key is pressed, and returns
                     // the true/false result.
} // Ends the isKeyPressed function.

bool InputManager::isKeyJustPressed(
    int key) const { // Defines the function checking for a tap/single press.
  if (key < 32 || key > GLFW_KEY_LAST)
    return false; // Safety check: if the key asked for is out of the valid
                  // range, immediately return false.
  bool current = (glfwGetKey(m_window, key) ==
                  GLFW_PRESS); // Asks GLFW if the key is down right NOW.
  bool previous = m_previousKeys[key]; // Looks up in our array to see if the
                                       // key was down the LAST frame.
  return current &&
         !previous; // Returns true ONLY if the key is down NOW (current), and
                    // was NOT down BEFORE (!previous).
} // Ends the isKeyJustPressed function.

bool InputManager::isKeyJustReleased(
    int key) const { // Defines the function checking for a key release.
  if (key < 32 || key > GLFW_KEY_LAST)
    return false; // Safety check: if the key asked for is out of the valid
                  // range, immediately return false.
  bool current = (glfwGetKey(m_window, key) ==
                  GLFW_PRESS); // Asks GLFW if the key is down right NOW.
  bool previous = m_previousKeys[key]; // Looks up in our array to see if the
                                       // key was down the LAST frame.
  return !current && previous; // Returns true ONLY if the key is NOT down NOW
                               // (!current), but WAS down BEFORE (previous).
} // Ends the isKeyJustReleased function.

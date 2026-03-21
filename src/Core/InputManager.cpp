#include "Core/InputManager.h"
#include <cstring>

InputManager* InputManager::s_instance = nullptr;

InputManager::InputManager() 
    : m_window(nullptr) {
    memset(m_previousKeys, 0, sizeof(m_previousKeys));
}

InputManager* InputManager::getInstance() {
    if (!s_instance) {
        s_instance = new InputManager();
    }
    return s_instance;
}

void InputManager::init(GLFWwindow* window) {
    // Store a reference to the main window for querying keys later
    m_window = window;
    // Clear out any garbage values from previous keys array
    memset(m_previousKeys, 0, sizeof(m_previousKeys));
}

void InputManager::update() {
    // Traverse all valid possible keys to save their state from the PREVIOUS frame.
    // GLFW keys start from GLFW_KEY_SPACE (32). Below 32 are invalid.
    for (int key = 32; key <= GLFW_KEY_LAST; key++) {
        m_previousKeys[key] = (glfwGetKey(m_window, key) == GLFW_PRESS);
    }
}

bool InputManager::isKeyPressed(int key) const {
    if (key < 32 || key > GLFW_KEY_LAST) return false;
    // Returns true if the key is held down right now
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool InputManager::isKeyJustPressed(int key) const {
    if (key < 32 || key > GLFW_KEY_LAST) return false;
    // Returns true if the key is down now, but wasn't in the previous frame
    bool current = (glfwGetKey(m_window, key) == GLFW_PRESS);
    bool previous = m_previousKeys[key];
    return current && !previous;
}

bool InputManager::isKeyJustReleased(int key) const {
    if (key < 32 || key > GLFW_KEY_LAST) return false;
    // Returns true if the key is up now, but was down in the previous frame
    bool current = (glfwGetKey(m_window, key) == GLFW_PRESS);
    bool previous = m_previousKeys[key];
    return !current && previous;
}
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
    // Traverse all possible keys to save their state from the PREVIOUS frame.
    // This MUST be called before glfwPollEvents() so that we save the old
    // input values before they get updated with the new events this frame.
    for (int key = 0; key <= GLFW_KEY_LAST; key++) {
        m_previousKeys[key] = (glfwGetKey(m_window, key) == GLFW_PRESS);
    }
}

bool InputManager::isKeyPressed(int key) const {
    // Returns true if the key is held down right now
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool InputManager::isKeyJustPressed(int key) const {
    // Returns true if the key is down now, but wasn't in the previous frame
    bool current = (glfwGetKey(m_window, key) == GLFW_PRESS);
    bool previous = m_previousKeys[key];
    return current && !previous;
}

bool InputManager::isKeyJustReleased(int key) const {
    // Returns true if the key is up now, but was down in the previous frame
    bool current = (glfwGetKey(m_window, key) == GLFW_PRESS);
    bool previous = m_previousKeys[key];
    return !current && previous;
}
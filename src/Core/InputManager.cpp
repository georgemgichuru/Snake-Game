#include "Core/InputManager.h"
#include <iostream>

InputManager* InputManager::s_instance = nullptr;

InputManager::InputManager() 
    : m_window(nullptr) {
}

InputManager* InputManager::getInstance() {
    if (!s_instance) {
        s_instance = new InputManager();
    }
    return s_instance;
}

void InputManager::init(GLFWwindow* window) {
    m_window = window;
    
    // Set callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    
    // Initialize key states
    for (int key = 0; key < 512; key++) {
        m_currentKeys[key] = false;
        m_previousKeys[key] = false;
    }
}

void InputManager::update() {
    // Copy current keys to previous
    m_previousKeys = m_currentKeys;
    
    // Update current key states
    for (int key = 0; key < 512; key++) {
        m_currentKeys[key] = glfwGetKey(m_window, key) == GLFW_PRESS;
    }
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // This is called by GLFW, we could handle immediate actions here
    // But we'll rely on the update method for simplicity
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // Handle mouse button input if needed
}

bool InputManager::isKeyPressed(int key) const {
    return m_currentKeys[key];
}

bool InputManager::isKeyJustPressed(int key) const {
    return m_currentKeys[key] && !m_previousKeys[key];
}

bool InputManager::isKeyJustReleased(int key) const {
    return !m_currentKeys[key] && m_previousKeys[key];
}
#pragma once

#include <functional>
#include <glm/glm.hpp>
#include "Core/Renderer.h"
#include "Core/InputManager.h"
#include "GameObjects/Snake.h"

/**
 * GameState - Manages the main game logic
 */
class GameState {
private:
    Renderer* m_renderer;
    InputManager* m_input;
    
    // Game objects
    Snake m_snake;
    
    // Food
    glm::vec2 m_foodPosition;
    glm::vec4 m_foodColor;
    
    // Game settings
    int m_gridWidth;
    int m_gridHeight;
    float m_cellSize;
    glm::vec2 m_gridOffset;
    
    // Game state
    bool m_isGameOver;
    bool m_isPaused;
    int m_score;
    
    // Callbacks
    std::function<void()> m_playCallback;
    std::function<void()> m_gameOverCallback;
    
    // Helper functions
    void generateFood();
    bool checkWallCollision(const glm::vec2& position);
    bool checkSelfCollision();
    
public:
    GameState(Renderer* renderer, InputManager* input);
    ~GameState() = default;
    
    // State lifecycle
    void enter();
    void exit();
    void update(float dt);
    void render();
    
    // Setters
    void setPlayCallback(std::function<void()> callback) { m_playCallback = callback; }
    void setGameOverCallback(std::function<void()> callback) { m_gameOverCallback = callback; }
    
    // Getters
    bool isGameOver() const { return m_isGameOver; }
    int getScore() const { return m_score; }
};
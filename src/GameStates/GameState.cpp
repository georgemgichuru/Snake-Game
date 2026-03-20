#include "GameStates/GameState.h"
#include <iostream>
#include <random>

GameState::GameState(Renderer* renderer, InputManager* input)
    : m_renderer(renderer)
    , m_input(input)
    , m_gridWidth(20)
    , m_gridHeight(15)
    , m_cellSize(40.0f)
    , m_isGameOver(false)
    , m_isPaused(false)
    , m_score(0) {
    
    // Calculate grid offset to center the game
    float totalWidth = m_gridWidth * m_cellSize;
    float totalHeight = m_gridHeight * m_cellSize;
    m_gridOffset = glm::vec2(
        (m_renderer->getScreenWidth() - totalWidth) / 2.0f,
        (m_renderer->getScreenHeight() - totalHeight) / 2.0f
    );
    
    m_foodColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red food
}

void GameState::enter() {
    // Initialize snake at center of grid
    Position startPos(m_gridWidth / 2, m_gridHeight / 2);
    m_snake.init(startPos, 3);
    
    // Reset game state
    m_isGameOver = false;
    m_isPaused = false;
    m_score = 0;
    
    // Generate first food
    generateFood();
}

void GameState::exit() {
    // Cleanup if needed
}

void GameState::update(float dt) {
    if (m_isGameOver || m_isPaused) {
        return;
    }
    
    // Handle input
    if (m_input->isKeyJustPressed(GLFW_KEY_UP)) {
        m_snake.setDirection(Direction::UP);
    } else if (m_input->isKeyJustPressed(GLFW_KEY_DOWN)) {
        m_snake.setDirection(Direction::DOWN);
    } else if (m_input->isKeyJustPressed(GLFW_KEY_LEFT)) {
        m_snake.setDirection(Direction::LEFT);
    } else if (m_input->isKeyJustPressed(GLFW_KEY_RIGHT)) {
        m_snake.setDirection(Direction::RIGHT);
    }
    
    // Pause game
    if (m_input->isKeyJustPressed(GLFW_KEY_P)) {
        m_isPaused = true;
        return;
    }
    
    // Move snake (using fixed timestep for consistent movement)
    static float accumulator = 0.0f;
    accumulator += dt;
    const float moveInterval = 0.15f;  // Move every 150ms
    
    while (accumulator >= moveInterval) {
        m_snake.move();
        accumulator -= moveInterval;
        
        // Check for food collision
        if (m_snake.getHead().x == m_foodPosition.x && m_snake.getHead().y == m_foodPosition.y) {
            m_snake.grow();
            m_score++;
            generateFood();
        }
        
        // Check for collisions
        if (checkWallCollision(m_snake.getHead()) || m_snake.checkSelfCollision()) {
            m_isGameOver = true;
            if (m_gameOverCallback) {
                m_gameOverCallback();
            }
            break;
        }
    }
}

void GameState::render() {
    // Draw background
    m_renderer->drawRect(
        glm::vec2(0, 0),
        glm::vec2(m_renderer->getScreenWidth(), m_renderer->getScreenHeight()),
        glm::vec4(0.1f, 0.1f, 0.1f, 1.0f)  // Dark gray background
    );
    
    // Draw grid lines (optional)
    for (int i = 0; i <= m_gridWidth; i++) {
        m_renderer->drawRect(
            glm::vec2(m_gridOffset.x + i * m_cellSize, m_gridOffset.y),
            glm::vec2(1.0f, m_gridHeight * m_cellSize),
            glm::vec4(0.2f, 0.2f, 0.2f, 0.5f)
        );
    }
    for (int i = 0; i <= m_gridHeight; i++) {
        m_renderer->drawRect(
            glm::vec2(m_gridOffset.x, m_gridOffset.y + i * m_cellSize),
            glm::vec2(m_gridWidth * m_cellSize, 1.0f),
            glm::vec4(0.2f, 0.2f, 0.2f, 0.5f)
        );
    }
    
    // Draw snake
    for (const auto& segment : m_snake.getSegments()) {
        glm::vec2 pos = m_gridOffset + glm::vec2(segment.x * m_cellSize, segment.y * m_cellSize);
        m_renderer->drawRect(pos, glm::vec2(m_cellSize, m_cellSize), glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
    }
    
    // Draw food
    glm::vec2 foodPos = m_gridOffset + glm::vec2(m_foodPosition.x * m_cellSize, m_foodPosition.y * m_cellSize);
    m_renderer->drawRect(foodPos, glm::vec2(m_cellSize, m_cellSize), m_foodColor);
    
    // Draw score (simplified - you'd want actual text rendering)
    // For now, we'll just use a placeholder
    
    if (m_isPaused) {
        // Draw pause text (placeholder)
    }
    
    if (m_isGameOver) {
        // Draw game over text (placeholder)
    }
}

void GameState::generateFood() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> disX(0, m_gridWidth - 1);
    std::uniform_int_distribution<> disY(0, m_gridHeight - 1);
    
    // Generate food at random position not occupied by snake
    do {
        m_foodPosition.x = disX(gen);
        m_foodPosition.y = disY(gen);
    } while (m_snake.checkCollision(m_foodPosition));
}

bool GameState::checkWallCollision(const glm::vec2& position) {
    return position.x < 0 || position.x >= m_gridWidth ||
           position.y < 0 || position.y >= m_gridHeight;
}
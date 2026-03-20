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
    if (m_isGameOver) {
        return;
    }
    
    if (m_isPaused) {
        // Check for unpause
        if (m_input->isKeyJustPressed(GLFW_KEY_P)) {
            m_isPaused = false;
        }
        return;
    }
    
    // Handle input - update direction based on key presses
    if (m_input->isKeyJustPressed(GLFW_KEY_UP)) {
        m_snake.setDirection(Direction::UP);
    } else if (m_input->isKeyJustPressed(GLFW_KEY_DOWN)) {
        m_snake.setDirection(Direction::DOWN);
    } else if (m_input->isKeyJustPressed(GLFW_KEY_LEFT)) {
        m_snake.setDirection(Direction::LEFT);
    } else if (m_input->isKeyJustPressed(GLFW_KEY_RIGHT)) {
        m_snake.setDirection(Direction::RIGHT);
    }
    
    // Handle pause
    if (m_input->isKeyJustPressed(GLFW_KEY_P)) {
        m_isPaused = true;
        return;
    }
    
    // Fixed timestep movement
    static float movementTimer = 0.0f;
    movementTimer += dt;
    
    // Move every 0.15 seconds (adjust this value to change speed)
    const float MOVE_INTERVAL = 0.15f;
    
    while (movementTimer >= MOVE_INTERVAL) {
        // Move the snake
        m_snake.move();
        movementTimer -= MOVE_INTERVAL;
        
        // Check for food collision
        if (m_snake.getHead().x == m_foodPosition.x && 
            m_snake.getHead().y == m_foodPosition.y) {
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
    // 1. Draw the main playfield background (a slightly lighter dark grey)
    m_renderer->drawRect(
        m_gridOffset,
        glm::vec2(m_gridWidth * m_cellSize, m_gridHeight * m_cellSize),
        glm::vec4(0.12f, 0.12f, 0.15f, 1.0f)
    );
    
    // 2. Draw subtle grid lines to make the cells visible
    for (int i = 0; i <= m_gridWidth; i++) {
        m_renderer->drawRect(
            glm::vec2(m_gridOffset.x + i * m_cellSize, m_gridOffset.y),
            glm::vec2(1.0f, m_gridHeight * m_cellSize),
            glm::vec4(0.2f, 0.2f, 0.25f, 0.4f)
        );
    }
    for (int i = 0; i <= m_gridHeight; i++) {
        m_renderer->drawRect(
            glm::vec2(m_gridOffset.x, m_gridOffset.y + i * m_cellSize),
            glm::vec2(m_gridWidth * m_cellSize, 1.0f),
            glm::vec4(0.2f, 0.2f, 0.25f, 0.4f)
        );
    }
    
    // 3. Draw the snake segments
    const auto& segments = m_snake.getSegments();
    for (size_t i = 0; i < segments.size(); i++) {
        const auto& segment = segments[i];
        
        // Calculate raw position for the cell
        glm::vec2 pos = m_gridOffset + glm::vec2(segment.x * m_cellSize, segment.y * m_cellSize);
        
        // Add a 1-pixel gap/margin between segments so it doesn't look like a solid blob
        glm::vec2 insetPos = pos + glm::vec2(2.0f, 2.0f);
        glm::vec2 insetSize = glm::vec2(m_cellSize - 4.0f, m_cellSize - 4.0f);
        
        // Give the head a distinct, brighter color to make direction obvious
        if (i == 0) {
            // Head color (bright neon green)
            m_renderer->drawRect(insetPos, insetSize, glm::vec4(0.2f, 1.0f, 0.2f, 1.0f));
        } else {
            // Body color (darker green)
            m_renderer->drawRect(insetPos, insetSize, glm::vec4(0.1f, 0.6f, 0.1f, 1.0f));
        }
    }
    
    // 4. Draw the food with a distinct look (red, with same inset)
    glm::vec2 foodPos = m_gridOffset + glm::vec2(m_foodPosition.x * m_cellSize, m_foodPosition.y * m_cellSize);
    // Draw an outer ring for a "glow" style
    m_renderer->drawRect(foodPos + glm::vec2(4.0f, 4.0f), glm::vec2(m_cellSize - 8.0f, m_cellSize - 8.0f), m_foodColor);
    
    // 5. Draw screen overlay states (Pause / Game Over)
    if (m_isPaused) {
        // Draw a dark semi-transparent overlay
        m_renderer->drawRect(
            glm::vec2(0, 0),
            glm::vec2(m_renderer->getScreenWidth(), m_renderer->getScreenHeight()),
            glm::vec4(0.0f, 0.0f, 0.0f, 0.5f)
        );
    }
    
    if (m_isGameOver) {
        // Draw a reddish semi-transparent overlay
        m_renderer->drawRect(
            glm::vec2(0, 0),
            glm::vec2(m_renderer->getScreenWidth(), m_renderer->getScreenHeight()),
            glm::vec4(0.5f, 0.0f, 0.0f, 0.5f)
        );
    }
}

void GameState::generateFood() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> disX(0, m_gridWidth - 1);
    std::uniform_int_distribution<int> disY(0, m_gridHeight - 1);
    
    // Generate food at random position not occupied by snake
    do {
        m_foodPosition.x = static_cast<float>(disX(gen));
        m_foodPosition.y = static_cast<float>(disY(gen));
    } while (m_snake.checkCollision(m_foodPosition));
}

bool GameState::checkWallCollision(const glm::vec2& position) {
    return position.x < 0 || position.x >= m_gridWidth ||
           position.y < 0 || position.y >= m_gridHeight;
}
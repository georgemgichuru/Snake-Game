#pragma once

#include <deque>
#include <glm/glm.hpp>

using Position = glm::vec2;

enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

/**
 * Snake - Manages snake logic and movement
 */
class Snake {
private:
    std::deque<Position> m_segments;
    Direction m_direction;
    Direction m_nextDirection;
    bool m_growPending;
    
public:
    Snake();
    ~Snake() = default;
    
    // Initialize snake at starting position
    void init(const Position& startPos, int initialLength);
    
    // Set direction (with validation)
    void setDirection(Direction dir);
    
    // Move snake based on current direction
    void move();
    
    // Schedule growth for next move
    void grow();
    
    // Check if snake collides with a given position
    bool checkCollision(const Position& pos) const;
    
    // Check if snake collides with itself
    bool checkSelfCollision() const;
    
    // Getters
    const std::deque<Position>& getSegments() const { return m_segments; }
    Position getHead() const { return m_segments.front(); }
    Direction getDirection() const { return m_direction; }
};
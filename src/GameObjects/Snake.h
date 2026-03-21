#pragma once

#include <deque>
#include <glm/glm.hpp>

// Easy-to-read name alias for 2D vectors
using Position = glm::vec2;

// The four allowed facings for the snake.
enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

/**
 * Snake Class:
 * Manages the data and logical grid movement representing the Snake player.
 * Uses a std::deque (Double Ended Queue) because a snake naturally works by:
 * 1) Popping the tail element constantly (back)
 * 2) Pushing a new head element constantly (front)
 */
class Snake {
private:
    std::deque<Position> m_segments; // Array of grid coordinates making up the body
    Direction m_direction;           // The direction we are CURRENTLY moving this exact tile
    std::deque<Direction> m_directionQueue; // Input queue to prevent button-mashing bugs and suicide U-turns
    bool m_growPending;              // If true, next time we move we DON'T remove the tail (effectively growing +1 unit)
    
public:
    Snake();
    ~Snake() = default;
    
    // Spawn the snake logically on the grid
    void init(const Position& startPos, int initialLength);
    
    // Asks the snake to turn. Validates logic (No immediate U-turns into your own neck)
    void setDirection(Direction dir);
    
    // Advances the deque one grid unit forward in whichever direction it's facing
    void move();
    
    // "Eats food". Signals that next `move()` should leave the tail alone.
    void grow();
    
    // Mathematics to check if this grid position perfectly matches ANY snake body part
    bool checkCollision(const Position& pos) const;
    
    // Iterates the body to see if the Head coordinate crashed into another Body coordinate
    bool checkSelfCollision() const;
    
    // Read-only getters so external systems (like Renderer) can ask where the snake is
    const std::deque<Position>& getSegments() const { return m_segments; }
    Position getHead() const { return m_segments.front(); }
    Direction getDirection() const { return m_direction; }
};
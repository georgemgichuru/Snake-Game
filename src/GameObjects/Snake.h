#pragma once

#include <deque> // Includes the Double-Ended Queue standard library
#include <glm/glm.hpp>

// Creates an easy-to-read alias so we can type 'Position' instead of 'glm::vec2'
using Position = glm::vec2;

// The four absolute directions the snake can travel on the grid
enum class Direction {
    UP,
    DOWN,
    LEFT,
    RIGHT
};

/**
 * Snake Class:
 * Manages the data and logical grid movement representing the Snake player.
 */
class Snake {
private:
    std::deque<Position> m_segments; // Array of grid coordinates making up the physical body
    Direction m_direction;           // The direction the snake is CURRENTLY moving this exact tile
    
    // A buffer storing up to 3 player inputs to prevent fast button-mashing from causing instant suicide U-turns
    std::deque<Direction> m_directionQueue; 
    
    // If true, the next time we step forward, we DON'T remove the tail (effectively growing +1 unit long)
    bool m_growPending;              
    
public:
    Snake();
    ~Snake() = default;
    
    // Spawns the snake logically on the grid at a specific XY coordinate
    void init(const Position& startPos, int initialLength);
    
    // Asks the snake to turn. Validates logic so you can't reverse into yourself
    void setDirection(Direction dir);
    
    // Advances the entire snake one grid unit forward
    void move();
    
    // Signals that the next `move()` should leave the tail alone, making the snake longer
    void grow();
    
    // Checks if the given 'pos' coordinate perfectly overlaps with ANY of the snake's body parts
    bool checkCollision(const Position& pos) const;
    
    // Checks if the Head coordinate crashed into any of its own trailing body coordinates
    bool checkSelfCollision() const;
    
    // --- GETTERS ---
    // Read-only functions so external systems (like the Renderer) can ask where the snake is
    const std::deque<Position>& getSegments() const { return m_segments; }
    Position getHead() const { return m_segments.front(); }
    Direction getDirection() const { return m_direction; }
};
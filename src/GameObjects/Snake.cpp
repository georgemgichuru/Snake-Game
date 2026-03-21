#include "GameObjects/Snake.h"
#include <algorithm>

Snake::Snake() 
    : m_direction(Direction::RIGHT)
    , m_growPending(false) {
    // Initial state constructor
}

void Snake::init(const Position& startPos, int initialLength) {
    // Clear out any old segments from previous runs
    m_segments.clear();
    m_directionQueue.clear();
    
    // Reset starting directions
    m_direction = Direction::RIGHT;
    m_growPending = false;
    
    // Create snake body segments stretching to the left
    for (int i = 0; i < initialLength; ++i) {
        m_segments.push_back(Position(startPos.x - i, startPos.y));
    }
}

void Snake::setDirection(Direction dir) {
    // Evaluate the last input direction queued, or the current direction if queue is empty
    Direction lastDir = m_directionQueue.empty() ? m_direction : m_directionQueue.back();

    // Prevent 180-degree turns (cannot instantly reverse into itself)
    if ((lastDir == Direction::UP && dir == Direction::DOWN) ||
        (lastDir == Direction::DOWN && dir == Direction::UP) ||
        (lastDir == Direction::LEFT && dir == Direction::RIGHT) ||
        (lastDir == Direction::RIGHT && dir == Direction::LEFT)) {
        return; // Ignore invalid input
    }
    
    // Queue up the direction change for the next movement ticks, capped to prevent infinite queue buildup
    if (m_directionQueue.size() < 3) {
        m_directionQueue.push_back(dir);
    }
}

void Snake::move() {
    // Commit the queued direction
    if (!m_directionQueue.empty()) {
        m_direction = m_directionQueue.front();
        m_directionQueue.pop_front();
    }
    
    // Calculate new head position based on current head
    Position newHead = m_segments.front();
    
    // Advance head one unit in the current direction
    switch (m_direction) {
        case Direction::UP:
            newHead.y -= 1;
            break;
        case Direction::DOWN:
            newHead.y += 1;
            break;
        case Direction::LEFT:
            newHead.x -= 1;
            break;
        case Direction::RIGHT:
            newHead.x += 1;
            break;
    }
    
    // Add new head to the front of the body
    m_segments.push_front(newHead);
    
    // If we're not growing this turn, remove the tail bit
    // This gives the illusion of movement
    if (!m_growPending) {
        m_segments.pop_back();
    } else {
        // Snake grew, reset the flag so we don't grow forever
        m_growPending = false;
    }
}

void Snake::grow() {
    // Signal the snake to not pop the tail on the next move
    m_growPending = true;
}

bool Snake::checkCollision(const Position& pos) const {
    // Check if the given position perfectly overlaps with ANY snake segment
    for (const auto& segment : m_segments) {
        if (segment.x == pos.x && segment.y == pos.y) {
            return true;
        }
    }
    return false;
}

bool Snake::checkSelfCollision() const {
    // The head is always index 0
    const Position& head = m_segments.front();
    
    // Check if head collides with any trailing body segment (skip index 0)
    for (size_t i = 1; i < m_segments.size(); ++i) {
        if (m_segments[i].x == head.x && m_segments[i].y == head.y) {
            return true;
        }
    }
    return false;
}
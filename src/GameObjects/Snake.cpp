#include "GameObjects/Snake.h"
#include <algorithm>

Snake::Snake() 
    : m_direction(Direction::RIGHT)
    , m_nextDirection(Direction::RIGHT)
    , m_growPending(false) {
}

void Snake::init(const Position& startPos, int initialLength) {
    m_segments.clear();
    m_direction = Direction::RIGHT;
    m_nextDirection = Direction::RIGHT;
    m_growPending = false;
    
    // Create snake body segments
    for (int i = 0; i < initialLength; ++i) {
        m_segments.push_back(Position(startPos.x - i, startPos.y));
    }
}

void Snake::setDirection(Direction dir) {
    // Prevent 180-degree turns
    if ((m_direction == Direction::UP && dir == Direction::DOWN) ||
        (m_direction == Direction::DOWN && dir == Direction::UP) ||
        (m_direction == Direction::LEFT && dir == Direction::RIGHT) ||
        (m_direction == Direction::RIGHT && dir == Direction::LEFT)) {
        return;
    }
    
    m_nextDirection = dir;
}

void Snake::move() {
    // Apply the next direction
    m_direction = m_nextDirection;
    
    // Calculate new head position
    Position newHead = m_segments.front();
    
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
    
    // Add new head
    m_segments.push_front(newHead);
    
    // Remove tail if not growing
    if (!m_growPending) {
        m_segments.pop_back();
    } else {
        m_growPending = false;
    }
}

void Snake::grow() {
    m_growPending = true;
}

bool Snake::checkCollision(const Position& pos) const {
    for (const auto& segment : m_segments) {
        if (segment.x == pos.x && segment.y == pos.y) {
            return true;
        }
    }
    return false;
}

bool Snake::checkSelfCollision() const {
    const Position& head = m_segments.front();
    
    // Check if head collides with any body segment (skip head itself)
    for (size_t i = 1; i < m_segments.size(); ++i) {
        if (m_segments[i].x == head.x && m_segments[i].y == head.y) {
            return true;
        }
    }
    return false;
}
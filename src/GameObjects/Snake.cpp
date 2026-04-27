#include "GameObjects/Snake.h"
#include <algorithm> // Included for general standard library algorithms (though not strictly used in this snippet)

Snake::Snake() 
    : m_direction(Direction::RIGHT) // Default starting direction is to the right
    , m_growPending(false) {        // We don't start the game needing to grow
}

void Snake::init(const Position& startPos, int initialLength) {
    // Clear out any old segments and inputs from previous rounds (so restarting the game works properly)
    m_segments.clear();
    m_directionQueue.clear();
    
    // Reset starting states
    m_direction = Direction::RIGHT;
    m_growPending = false;
    
    // Create the initial snake body.
    // Since it's moving RIGHT, the head is at startPos, and the body stretches out to the LEFT (startPos.x - i).
    for (int i = 0; i < initialLength; ++i) {
        m_segments.push_back(Position(startPos.x - i, startPos.y));
    }
}

void Snake::setDirection(Direction dir) {
    // Look at what the last queued direction was. If the queue is empty, just look at the physical current direction.
    Direction lastDir = m_directionQueue.empty() ? m_direction : m_directionQueue.back();

    // Prevent impossible 180-degree turns. If we are going UP, we cannot instantly go DOWN.
    if ((lastDir == Direction::UP && dir == Direction::DOWN) ||
        (lastDir == Direction::DOWN && dir == Direction::UP) ||
        (lastDir == Direction::LEFT && dir == Direction::RIGHT) ||
        (lastDir == Direction::RIGHT && dir == Direction::LEFT)) {
        return; // It's an illegal move, so exit the function and ignore the input entirely.
    }
    
    // If the move is valid, add it to our input buffer.
    // We cap it at 3 so a player can't mash 50 keys while paused and watch the snake do a crazy dance later.
    if (m_directionQueue.size() < 3) {
        m_directionQueue.push_back(dir);
    }
}

void Snake::move() {
    // 1. Check if the player queued up a turn. If so, grab the oldest one and make it our current direction.
    if (!m_directionQueue.empty()) {
        m_direction = m_directionQueue.front();
        m_directionQueue.pop_front(); // Remove it from the queue now that we've committed to it.
    }
    
    // 2. Look at the current head position
    Position newHead = m_segments.front();
    
    // 3. Shift the 'newHead' coordinate 1 unit in whichever direction we are facing
    switch (m_direction) {
        case Direction::UP:
            newHead.y -= 1; // Y goes UP as the number gets smaller (Top-Left Origin math!)
            break;
        case Direction::DOWN:
            newHead.y += 1; // Y goes DOWN as the number gets bigger
            break;
        case Direction::LEFT:
            newHead.x -= 1;
            break;
        case Direction::RIGHT:
            newHead.x += 1;
            break;
    }
    
    // 4. Attach the calculated new head to the very front of the snake's body
    m_segments.push_front(newHead);
    
    // 5. Handle the tail
    if (!m_growPending) {
        // We didn't eat an apple, so delete the last tail segment.
        // Because we added a head and removed a tail, the snake looks like it physically slithered forward 1 space!
        m_segments.pop_back(); 
    } else {
        // We DID eat an apple! We keep the tail where it is.
        // Because we added a new head but kept the old tail, the snake is exactly 1 block longer.
        m_growPending = false; // Reset the flag so we don't grow infinitely forever
    }
}

void Snake::grow() {
    // All this does is flip the flag. The actual math happens in the next move() cycle!
    m_growPending = true;
}

bool Snake::checkCollision(const Position& pos) const {
    // Loop through every single coordinate inside our snake's body
    for (const auto& segment : m_segments) {
        // If the X and Y coordinates perfectly match the position we are asking about...
        if (segment.x == pos.x && segment.y == pos.y) {
            return true; // We found a collision!
        }
    }
    return false; // The space is clear
}

bool Snake::checkSelfCollision() const {
    // The head is always safely sitting at index 0 of our deque
    const Position& head = m_segments.front();
    
    // Loop through the rest of the body, starting at index 1 (we don't want the head to check if it's colliding with itself!)
    for (size_t i = 1; i < m_segments.size(); ++i) {
        if (m_segments[i].x == head.x && m_segments[i].y == head.y) {
            return true; // The head mathematically overlapped with a body segment. Game Over!
        }
    }
    return false; // The snake is safe
}
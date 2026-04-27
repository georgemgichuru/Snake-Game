#pragma once

#include <chrono> // Standard C++ library for tracking high-precision time

/**
 * Time Class (Singleton):
 * Manages the game's internal stopwatch. Its primary job is to calculate 'Delta Time' (dt).
 * Delta Time ensures that animations and movements happen at the same real-world speed 
 * regardless of whether the player's computer is fast or slow.
 */
class Time {
private:
    static Time* s_instance; // The single global instance
    
    float m_deltaTime;      // Time elapsed since the last frame (in fractions of a second)
    float m_fixedDeltaTime; // A constant time step usually used for physics updates
    float m_time;           // Total time elapsed since the game started

    // A high-precision timestamp of exactly when the previous frame occurred
    std::chrono::high_resolution_clock::time_point m_lastFrame;
    
    Time(); // Private constructor enforces the Singleton pattern
    ~Time() = default;
    
public:
    // Delete copy/assignment to strictly prevent duplicating the clock
    Time(const Time&) = delete;
    Time& operator=(const Time&) = delete;
    
    static Time* getInstance();
    
    // Initializes the clock (called once at the exact start of the game)
    void init();
    
    // Calculates Delta Time and updates the clock (called exactly once per game frame)
    void update();
    
    // --- GETTERS ---
    // These allow other files to simply write `Time::getInstance()->deltaTime()`
    
    float deltaTime() const { return m_deltaTime; }
    float fixedDeltaTime() const { return m_fixedDeltaTime; }
    float time() const { return m_time; }
};
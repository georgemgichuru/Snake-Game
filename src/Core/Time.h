#pragma once

#include <chrono>

/**
 * Time Class:
 * Regulates the speed at which your game runs. Without it, a powerful computer
 * would run Snake so fast you'd die in 0.001 seconds.
 * Calculates `dt` (Delta Time), ensuring motion looks consistent on all monitors.
 */
class Time {
private:
    static Time* s_instance; // Singleton Architecture
    
    // deltaTime = "how many fractions of a second did the previous frame take?"
    // If you tell an object to move X pixels * deltaTime, it guarantees smooth real-world speed
    float m_deltaTime;
    
    // Fixed steps typically used if throwing physics calculations (like gravity) in
    float m_fixedDeltaTime;
    
    // Track total seconds alive since program start
    float m_time;
    
    // The timestamp recording the EXACT millisecond the last frame ticked
    std::chrono::high_resolution_clock::time_point m_lastFrame;
    
    Time();
    ~Time() = default;
    
public:
    // Prevent duplicated copying of this Singleton globally
    Time(const Time&) = delete;
    Time& operator=(const Time&) = delete;
    
    static Time* getInstance();
    
    // Prepares the internal stopwatch initially
    void init();
    
    // Calculates elapsed span and overwrites our last known timestamp (Call once per game loop!)
    void update();
    
    // Expose the calculated floating points for easy math in GameState.cpp
    float deltaTime() const { return m_deltaTime; }
    
    float fixedDeltaTime() const { return m_fixedDeltaTime; }
    
    float time() const { return m_time; }
};
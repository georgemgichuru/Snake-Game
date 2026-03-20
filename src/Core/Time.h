#pragma once

#include <chrono>

/**
 * Time class - Handles game timing and delta time calculations
 */
class Time {
private:
    static Time* s_instance;
    
    float m_deltaTime;
    float m_fixedDeltaTime;
    float m_time;
    std::chrono::high_resolution_clock::time_point m_lastFrame;
    
    Time();
    ~Time() = default;
    
public:
    // Delete copy constructor and assignment operator
    Time(const Time&) = delete;
    Time& operator=(const Time&) = delete;
    
    static Time* getInstance();
    
    // Initialize timing system
    void init();
    
    // Update delta time (call once per frame)
    void update();
    
    // Get delta time for current frame
    float deltaTime() const { return m_deltaTime; }
    
    // Get fixed delta time (for physics)
    float fixedDeltaTime() const { return m_fixedDeltaTime; }
    
    // Get total time since game start
    float time() const { return m_time; }
};
#include "Core/Time.h"

Time* Time::s_instance = nullptr;

Time::Time() 
    : m_deltaTime(0.0f)
    , m_fixedDeltaTime(0.016f)  // Target ~60 FPS logic ticks roughly.
    , m_time(0.0f) {
}

Time* Time::getInstance() {
    if (!s_instance) {
        s_instance = new Time();
    }
    return s_instance;
}

void Time::init() {
    // Read the OS CPU system clock and stamp it for right now essentially.
    m_lastFrame = std::chrono::high_resolution_clock::now();
}

void Time::update() {
    // 1) Read the current clock right now at this very slice of a second.
    auto currentTime = std::chrono::high_resolution_clock::now();
    
    // 2) Mathematically deduce the gap between `currentTime` and `m_lastFrame`
    std::chrono::duration<float> elapsed = currentTime - m_lastFrame;
    m_deltaTime = elapsed.count();
    
    // 3) Push `currentTime` into the `m_lastFrame` variable so we are ready for the NEXT loop measuring
    m_lastFrame = currentTime;
    
    // Safety cap: If you alt-tab the game or lag aggressively, `m_deltaTime` could spike huge (like 5.0 seconds).
    // This cap stops objects clipping straight through walls or exploding mathematically if the game stutters.
    if (m_deltaTime > 0.033f) {
        m_deltaTime = 0.033f;
    }
    
    // Continually add up the fractions to form whole seconds since application launch
    m_time += m_deltaTime;
}
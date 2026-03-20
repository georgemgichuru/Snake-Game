#include "Core/Time.h"

Time* Time::s_instance = nullptr;

Time::Time() 
    : m_deltaTime(0.0f)
    , m_fixedDeltaTime(0.016f)  // 60 FPS fixed timestep
    , m_time(0.0f) {
}

Time* Time::getInstance() {
    if (!s_instance) {
        s_instance = new Time();
    }
    return s_instance;
}

void Time::init() {
    m_lastFrame = std::chrono::high_resolution_clock::now();
}

void Time::update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - m_lastFrame;
    m_deltaTime = elapsed.count();
    m_lastFrame = currentTime;
    
    // Cap delta time to prevent large jumps
    if (m_deltaTime > 0.033f) {
        m_deltaTime = 0.033f;
    }
    
    m_time += m_deltaTime;
}
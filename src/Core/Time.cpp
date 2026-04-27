#include "Core/Time.h"

Time* Time::s_instance = nullptr;

Time::Time() 
    : m_deltaTime(0.0f)
    , m_fixedDeltaTime(0.016666f) // Exactly 1/60th of a second (Targeting 60 physics updates per sec)
    , m_time(0.0f) 
{
}

Time* Time::getInstance() {
    if (!s_instance) {
        s_instance = new Time();
    }
    return s_instance;
}

void Time::init() {
    // Record the exact current time to serve as the baseline for the very first frame
    m_lastFrame = std::chrono::high_resolution_clock::now();
}

void Time::update() {
    // 1) Check the PC's hardware clock right now
    auto currentTime = std::chrono::high_resolution_clock::now();
    
    // 2) Calculate the mathematical difference between now and the last frame
    std::chrono::duration<float> elapsed = currentTime - m_lastFrame;
    
    // Extract the duration as a standard floating-point number (in seconds)
    m_deltaTime = elapsed.count(); 
    
    // 3) Update the last frame marker so it is ready to be measured against in the NEXT loop
    m_lastFrame = currentTime;
    
    // DELTA TIME CAP (Spike Prevention):
    // If you grab the window to move it, or the PC lags heavily, a frame might take 1.0 full seconds.
    // If we multiply our snake's speed by 1.0s, it might instantly teleport straight through a wall!
    // By capping dt at 0.033f (equivalent to dropping to 30 FPS), we ensure physics mathematically 
    // never break, even if the game visually stutters for a moment.
    if (m_deltaTime > 0.033f) {
        m_deltaTime = 0.033f;
    }
    
    // Keep a running total of how long the game has been open in seconds
    m_time += m_deltaTime;
}
#pragma once

#include <functional> // Allows us to pass functions around as variables using std::function
#include <glm/glm.hpp>
#include "Core/Renderer.h"
#include "Core/InputManager.h"
#include "Core/ParticleSystem.h"
#include "GameObjects/Snake.h"
#include "soloud.h"     // The core audio engine library
#include "soloud_wav.h" // The module that loads .wav files

// An enum class defining the four distinct screens/states the game can be in.
enum class AppState {
    START,      // "Press ENTER to start" splash screen
    PLAYING,    // Normal gameplay
    PAUSED,     // Paused overlay
    GAME_OVER   // Death overlay with final score
};

/**
 * GameState Class:
 * Forms the core "Rules Engine" of the actual Snake Game concept. 
 * Doesn't handle raw OpenGL itself, instead delegating math and drawings
 * down to its `m_snake` or `m_renderer` objects while dictating the rules.
 */
class GameState {
private:
    Renderer* m_renderer;   // Reference to the drawing system
    InputManager* m_input;  // Reference to the hardware input system
    
    // ----------------------------------------------------
    // GAME OBJECTS & DATA
    // ----------------------------------------------------
    Snake m_snake;
    ParticleSystem m_particles;

    glm::vec2 m_foodPosition; // The X,Y grid coordinate of the apple
    glm::vec4 m_foodColor;    // The RGBA color profile for the apple
    
    // Board logic: How many cells wide/high the grid is, and pixel size per cell
    int m_gridWidth;
    int m_gridHeight;
    float m_cellSize;         // The width/height of a single tile in pixels
    glm::vec2 m_gridOffset;   // Pixel offset used to center the board inside the Window
    
    // Tracking flags
    AppState m_appState = AppState::START; // Start the game on the Splash screen
    bool m_isGameOver = false; 
    bool m_isPaused = false;
    int m_score = 0;
    int m_highScore = 0;
    
    // Callbacks: A way for this class to trigger functions in `main.cpp`
    std::function<void()> m_playCallback;
    std::function<void()> m_gameOverCallback;

    // SoLoud audio objects
    SoLoud::Soloud m_soloud;   // The master audio engine
    SoLoud::Wav m_eatSound;    // The sound effect for eating an apple
    SoLoud::Wav m_dieSound;    // The sound effect for crashing
    
    // ----------------------------------------------------
    // INTERNAL HELPERS
    // ----------------------------------------------------
    void generateFood(); 
    bool checkWallCollision(const glm::vec2& position);
    void resetGame();
    
public:
    // Constructor requires references to our core engine systems
    GameState(Renderer* renderer, InputManager* input);
    ~GameState() = default;
    
    // Phase hooks heavily utilized in the main game loop
    void enter();            // Bootstraps audio, fonts, and resets score upon launch.
    void exit();             // Safely shuts down external libraries (like SoLoud).
    void update(float dt);   // The beating heart of the game: checking timers, movement, deaths.
    void render();           // Visual instruction block passed down to OpenGL.
    
    // Registration commands to link code snippets from outside this class
    void setPlayCallback(std::function<void()> callback) { m_playCallback = callback; }
    void setGameOverCallback(std::function<void()> callback) { m_gameOverCallback = callback; }
    
    // Public Status checking
    bool isGameOver() const { return m_isGameOver; }
    int getScore() const { return m_score; }
};
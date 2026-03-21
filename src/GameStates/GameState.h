#pragma once

#include <functional>
#include <glm/glm.hpp>
#include "Core/Renderer.h"
#include "Core/InputManager.h"
#include "Core/ParticleSystem.h"
#include "GameObjects/Snake.h"
#include "soloud.h"
#include "soloud_wav.h"

// The four distinct screens the game can be in
enum class AppState {
    START,      // "Press ENTER to start" splash
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
    Renderer* m_renderer;   // Reference to drawing system
    InputManager* m_input;  // Reference to hardware tracking system
    
    // ----------------------------------------------------
    // GAME OBJECTS & DATA
    // ----------------------------------------------------
    Snake m_snake;
    ParticleSystem m_particles;

    
    glm::vec2 m_foodPosition; // Where is the food hiding on the grid?
    glm::vec4 m_foodColor;    // Color profile for the food item
    
    // Board logic: How many cells wide/high, and pixel size per cell
    int m_gridWidth;
    int m_gridHeight;
    float m_cellSize;         // Width/Height logic of a single tile
    glm::vec2 m_gridOffset;   // Pixel pushing to center the board inside the Window
    
    // Tracking flags
    AppState m_appState = AppState::START;
    bool m_isGameOver = false; 
    bool m_isPaused = false;
    int m_score = 0;
    int m_highScore = 0;
    
    // Optional Callbacks. Essentially functions we can fire back into `main.cpp` 
    // to do things like printing logs natively out there.
    std::function<void()> m_playCallback;
    std::function<void()> m_gameOverCallback;

    SoLoud::Soloud m_soloud;
    SoLoud::Wav m_eatSound;
    SoLoud::Wav m_dieSound;
    
    // ----------------------------------------------------
    // INTERNAL HELPERS
    // ----------------------------------------------------
    void generateFood();
    bool checkWallCollision(const glm::vec2& position);
    bool checkSelfCollision();
    void resetGame();
    
public:
    // Takes references to the engine layers so GameState can make demands of them
    GameState(Renderer* renderer, InputManager* input);
    ~GameState() = default;
    
    // Phase hooks heavily utilized in `main.cpp`.
    void enter();            // Sets everything to default, resets score, generates initial food.
    void exit();             // Halts operations, typically called right before reset/shutdown.
    void update(float dt);   // The beating heart of the game: checking timers, movement, deaths.
    void render();           // Visual instruction block passed rapidly down to OpenGL.
    
    // Registration commands to link Lambda code snippets from inside `main.cpp`
    void setPlayCallback(std::function<void()> callback) { m_playCallback = callback; }
    void setGameOverCallback(std::function<void()> callback) { m_gameOverCallback = callback; }
    
    // Status checking
    bool isGameOver() const { return m_isGameOver; }
    int getScore() const { return m_score; }
};
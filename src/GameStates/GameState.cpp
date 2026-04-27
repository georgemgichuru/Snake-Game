#include "GameStates/GameState.h"
#include <iostream>
#include <random> // Modern C++ hardware-based random number generator

// ==============================================================================
// AUTO-TILING HELPER FUNCTIONS (Hidden to this file only using the 'static' keyword)
// ==============================================================================

// Subtracting vector 'from' from vector 'to' gives us a direction vector.
// Example: To(5,0) - From(4,0) = (1,0) which means the direction is RIGHT.
static Direction directionBetween(const glm::vec2& from, const glm::vec2& to) {
    glm::vec2 d = to - from;
    if      (d.x >  0) return Direction::RIGHT;
    else if (d.x <  0) return Direction::LEFT;
    else if (d.y >  0) return Direction::DOWN;
    else               return Direction::UP;
}

// Simply maps the snake's current facing direction to the correct image file
static std::string headSprite(Direction dir) {
    switch (dir) {
        case Direction::UP:    return "head_up";
        case Direction::DOWN:  return "head_down";
        case Direction::LEFT:  return "head_left";
        case Direction::RIGHT: return "head_right";
    }
    return "head_right";
}

// The tail always points "away" from the body segment in front of it.
static std::string tailSprite(const glm::vec2& tail, const glm::vec2& beforeTail) {
    Direction toward = directionBetween(tail, beforeTail); // Where is the body relative to the tail?
    switch (toward) {
        case Direction::UP:    return "tail_down";  // Body is UP, so tail points DOWN
        case Direction::DOWN:  return "tail_up";    // Body is DOWN, so tail points UP
        case Direction::LEFT:  return "tail_right"; // etc...
        case Direction::RIGHT: return "tail_left";
    }
    return "tail_right";
}

// Determines if a middle-body segment is straight, or a corner, by looking at its two neighbors!
static std::string bodySprite(const glm::vec2& prev, const glm::vec2& curr, const glm::vec2& next) { 
    Direction toHead = directionBetween(curr, prev); // Direction to the segment closer to the head
    Direction toTail = directionBetween(curr, next); // Direction to the segment closer to the tail

    // If both neighbors are horizontally aligned with us, use a straight horizontal sprite
    bool horizontal = (toHead == Direction::LEFT  || toHead == Direction::RIGHT) &&
                      (toTail == Direction::LEFT  || toTail == Direction::RIGHT);
                      
    // If both neighbors are vertically aligned with us, use a straight vertical sprite
    bool vertical   = (toHead == Direction::UP    || toHead == Direction::DOWN)  &&
                      (toTail == Direction::UP    || toTail == Direction::DOWN);

    if (horizontal) return "body_horizontal";
    if (vertical)   return "body_vertical";

    // C++ LAMBDA: A temporary inline function!
    // It just checks if the two directions we calculated match the two required for a specific corner type.
    auto has = [&](Direction a, Direction b) {
        return (toHead == a && toTail == b) || (toHead == b && toTail == a);
    };
    
    // Map the directional results to the correct corner PNG file.
    if (has(Direction::LEFT,  Direction::UP))   return "body_topleft";
    if (has(Direction::RIGHT, Direction::UP))   return "body_topright";
    if (has(Direction::LEFT,  Direction::DOWN)) return "body_bottomleft";
    if (has(Direction::RIGHT, Direction::DOWN)) return "body_bottomright";

    return "body_horizontal"; // Fallback safety
}

// ==============================================================================
// GAMESTATE IMPLEMENTATION
// ==============================================================================

GameState::GameState(Renderer* renderer, InputManager* input)
    : m_renderer(renderer)
    , m_input(input)
    , m_particles(500)     // Pre-allocate 500 particles in our object pool
    , m_gridWidth(20)      // The board is 20 tiles wide
    , m_gridHeight(15)     // The board is 15 tiles tall
    , m_cellSize(40.0f)    // Each tile is 40x40 pixels visually
    , m_isGameOver(false)
    , m_isPaused(false)
    , m_score(0) {
    
    // Calculate exactly how many pixels to shove the grid right/down so it is perfectly centered in the window
    float totalWidth = m_gridWidth * m_cellSize;
    float totalHeight = m_gridHeight * m_cellSize;
    m_gridOffset = glm::vec2(
        (m_renderer->getScreenWidth() - totalWidth) / 2.0f,
        (m_renderer->getScreenHeight() - totalHeight) / 2.0f
    );
    
    m_foodColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);  // Solid Red apple
}

void GameState::enter() {
    // Bootstrap the SoLoud audio engine and load the sound files
    m_soloud.init();
    m_eatSound.load("res/assets/untitled.wav");
    m_dieSound.load("res/assets/untitled2.wav");

    m_appState = AppState::START; // Force state to the splash screen

    // Spawn the snake in the dead center of the grid
    Position startPos(m_gridWidth / 2, m_gridHeight / 2);
    m_snake.init(startPos, 3);
    
    m_isGameOver = false;
    m_isPaused   = false;
    m_score      = 0;
    m_highScore  = 0;
    generateFood(); // Place the first apple

    // Load every single PNG required for the snake into GPU memory via the ResourceManager
    auto* rm = ResourceManager::getInstance();
    const std::string base = "res/assets/Graphics/";
    for (auto& name : { "head_up","head_down","head_left","head_right",
                        "tail_up","tail_down","tail_left","tail_right",
                        "body_horizontal","body_vertical",
                        "body_topleft","body_topright",
                        "body_bottomleft","body_bottomright",
                        "apple" }) {
        rm->loadTexture(name, base + name + ".png");
    }

    // Initialize FreeType using the .ttf font file
    m_renderer->initText("res/assets/fonts/font.ttf", 28);
}

void GameState::exit() {
    m_soloud.deinit(); // Turn off audio before closing the game to prevent crashes
}

void GameState::resetGame() {
    Position startPos(m_gridWidth / 2, m_gridHeight / 2);
    m_snake.init(startPos, 3);
    m_isGameOver = false;
    m_isPaused   = false;
    m_score      = 0;
    generateFood();
    
    // Reset our static accumulator timer so the snake doesn't instantly double-jump when restarting
    static float movementTimer = 0.0f;
    movementTimer = 0.0f;
}

void GameState::update(float dt) {
    // Run different logic depending on what screen we are currently looking at
    switch (m_appState) {

    case AppState::START:
        if (m_input->isKeyJustPressed(GLFW_KEY_ENTER)) { // If they hit enter...
            resetGame();                                 // Reset values
            m_appState = AppState::PLAYING;              // Jump to playing logic!
        }
        break;

    case AppState::GAME_OVER:
        if (m_input->isKeyJustPressed(GLFW_KEY_R)) {
            resetGame();
            m_appState = AppState::PLAYING;
        }
        break;

    case AppState::PAUSED:
        if (m_input->isKeyJustPressed(GLFW_KEY_P)) {
            m_appState  = AppState::PLAYING; // Unpause!
            m_isPaused  = false;
        }
        break;

    case AppState::PLAYING: {
        // Cap dt to 100ms max — If the PC lags heavily, the snake won't teleport through walls.
        if (dt > 0.1f) dt = 0.1f;

        // Route inputs directly to the snake's input queue
        if (m_input->isKeyJustPressed(GLFW_KEY_UP))         m_snake.setDirection(Direction::UP);
        else if (m_input->isKeyJustPressed(GLFW_KEY_DOWN))  m_snake.setDirection(Direction::DOWN);
        else if (m_input->isKeyJustPressed(GLFW_KEY_LEFT))  m_snake.setDirection(Direction::LEFT);
        else if (m_input->isKeyJustPressed(GLFW_KEY_RIGHT)) m_snake.setDirection(Direction::RIGHT);

        if (m_input->isKeyJustPressed(GLFW_KEY_P)) {
            m_appState = AppState::PAUSED;
            m_isPaused = true;
            break;
        }

        // --- THE TIME ACCUMULATOR PATTERN ---
        static float movementTimer = 0.0f;
        movementTimer += dt;                 // Add the frame time to our total
        const float MOVE_INTERVAL = 0.15f;   // The snake moves exactly once every 0.15 seconds

        // If 0.15 seconds have passed, execute a grid movement!
        while (movementTimer >= MOVE_INTERVAL) {
            m_snake.move(); 
            movementTimer -= MOVE_INTERVAL; // Subtract 0.15, leaving any remainder intact!

            // Emit a single green trail particle at the snake's head every time it moves
            glm::vec2 headScreenPos = m_gridOffset + glm::vec2(m_snake.getHead().x * m_cellSize + m_cellSize / 2.0f,
                                                               m_snake.getHead().y * m_cellSize + m_cellSize / 2.0f);
            m_particles.Emit(1, headScreenPos, glm::vec2(0.0f), glm::vec4(0.2f, 1.0f, 0.2f, 0.5f));

            // Check if the snake's head perfectly overlaps the apple's coordinate
            if (m_snake.getHead().x == m_foodPosition.x && m_snake.getHead().y == m_foodPosition.y) {
                m_soloud.play(m_eatSound); // Play crunch sound
                
                // Explode 20 particles where the apple was!
                glm::vec2 foodWorldPos = m_gridOffset + glm::vec2(m_foodPosition.x * m_cellSize + m_cellSize / 2.0f,
                                                                  m_foodPosition.y * m_cellSize + m_cellSize / 2.0f);
                m_particles.Emit(20, foodWorldPos, glm::vec2(0.0f), m_foodColor);
                
                m_snake.grow(); // Tell the snake object to not delete its tail this turn
                m_score++;
                if (m_score > m_highScore) m_highScore = m_score; 
                generateFood(); // Spawn a new apple
            }

            // Check if the snake hit a wall, or hit its own tail
            if (checkWallCollision(m_snake.getHead()) || m_snake.checkSelfCollision()) {
                m_isGameOver = true;
                m_appState   = AppState::GAME_OVER;
                m_soloud.play(m_dieSound);
                if (m_gameOverCallback) m_gameOverCallback(); // Fire external callback if it exists
                break; // Break out of the while loop immediately
            }
        }

        m_particles.Update(dt); // Move all currently active particles by their velocities
        break;
    } // case PLAYING

    } // switch
}

void GameState::render() {
    // 1. Always draw the background grid first so it sits at the back.
    m_renderer->drawRect(
        m_gridOffset,
        glm::vec2(m_gridWidth * m_cellSize, m_gridHeight * m_cellSize),
        glm::vec4(0.12f, 0.12f, 0.15f, 1.0f) // Dark gray/blue background
    );

    // Draw the faint vertical and horizontal grid lines
    for (int i = 0; i <= m_gridWidth; i++) {
        m_renderer->drawRect(glm::vec2(m_gridOffset.x + i * m_cellSize, m_gridOffset.y),
                             glm::vec2(1.0f, m_gridHeight * m_cellSize),
                             glm::vec4(0.2f, 0.2f, 0.25f, 0.4f));
    }
    for (int i = 0; i <= m_gridHeight; i++) {
        m_renderer->drawRect(glm::vec2(m_gridOffset.x, m_gridOffset.y + i * m_cellSize),
                             glm::vec2(m_gridWidth * m_cellSize, 1.0f),
                             glm::vec4(0.2f, 0.2f, 0.25f, 0.4f));
    }

    // 2. Draw Snake + Food
    const auto& segments = m_snake.getSegments();
    auto* rm = ResourceManager::getInstance();

    for (size_t i = 0; i < segments.size(); i++) {
        // Convert grid coordinates (like 5,5) into actual screen pixels by multiplying by cell size
        glm::vec2 pos(m_gridOffset + glm::vec2(segments[i].x * m_cellSize, segments[i].y * m_cellSize));
        glm::vec2 sz(m_cellSize, m_cellSize);
        
        std::string spriteName; // Figure out exactly which PNG this piece of the body needs
        if (i == 0)
            spriteName = headSprite(m_snake.getDirection()); // Head
        else if (i == segments.size() - 1)
            spriteName = tailSprite(segments[i], segments[i - 1]); // Tail
        else
            spriteName = bodySprite(segments[i - 1], segments[i], segments[i + 1]); // Middle body

        auto tex = rm->getTexture(spriteName);
        if (tex) {
            m_renderer->drawSprite(tex, pos, sz); // If texture exists, draw it!
        } else {
            // Safety fallback: If PNG fails to load, just draw a green square.
            glm::vec2 ip = pos + glm::vec2(2.0f);
            glm::vec2 is = glm::vec2(m_cellSize - 4.0f);
            m_renderer->drawRect(ip, is, i == 0 ? glm::vec4(0.2f, 1.0f, 0.2f, 1.0f) : glm::vec4(0.1f, 0.6f, 0.1f, 1.0f));
        }
    }

    // Draw the Apple
    glm::vec2 foodPos = m_gridOffset + glm::vec2(m_foodPosition.x * m_cellSize, m_foodPosition.y * m_cellSize);
    auto appleTex = rm->getTexture("apple");
    if (appleTex)
        m_renderer->drawSprite(appleTex, foodPos, glm::vec2(m_cellSize));
    else
        m_renderer->drawRect(foodPos + glm::vec2(4.0f), glm::vec2(m_cellSize - 8.0f), m_foodColor);

    // Render any active explosions/trails
    m_particles.Draw(m_renderer);

    // 3. Draw UI
    float sw = static_cast<float>(m_renderer->getScreenWidth());

    // Only draw the Top Score bar if we are NOT on the splash screen
    if (m_appState == AppState::PLAYING || m_appState == AppState::PAUSED || m_appState == AppState::GAME_OVER) {
        m_renderer->drawText("SCORE  " + std::to_string(m_score), 20.0f, 10.0f, 0.8f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_renderer->drawText("BEST  " + std::to_string(m_highScore), sw - 180.0f, 10.0f, 0.8f, glm::vec4(0.8f, 0.8f, 0.2f, 1.0f));
    }

    // 4. Draw State-Specific Screen Overlays
    float sh = static_cast<float>(m_renderer->getScreenHeight());

    if (m_appState == AppState::START) {
        m_renderer->drawRect({0, 0}, {sw, sh}, {0.0f, 0.0f, 0.0f, 0.55f}); // Dark transparent background shade
        m_renderer->drawText("SNAKE", 0, sh * 0.30f, 2.0f, glm::vec4(0.2f, 1.0f, 0.2f, 1.0f), true);
        m_renderer->drawText("Press ENTER to Start", 0, sh * 0.50f, 0.9f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), true);
        m_renderer->drawText("Arrow keys to move   P to pause", 0, sh * 0.62f, 0.65f, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f), true);
    }
    if (m_appState == AppState::PAUSED) {
        m_renderer->drawRect({0, 0}, {sw, sh}, {0.0f, 0.0f, 0.0f, 0.5f});
        m_renderer->drawText("PAUSED", 0, sh * 0.40f, 1.6f, glm::vec4(1.0f, 1.0f, 0.2f, 1.0f), true);
        m_renderer->drawText("Press P to continue", 0, sh * 0.57f, 0.85f, glm::vec4(0.9f, 0.9f, 0.9f, 1.0f), true);
    }
    if (m_appState == AppState::GAME_OVER) {
        m_renderer->drawRect({0, 0}, {sw, sh}, {0.5f, 0.0f, 0.0f, 0.55f}); // Reddish transparent tint for death
        m_renderer->drawText("GAME OVER", 0, sh * 0.35f, 1.6f, glm::vec4(1.0f, 0.2f, 0.2f, 1.0f), true);
        m_renderer->drawText("Score: " + std::to_string(m_score), 0, sh * 0.52f, 1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f), true);
        m_renderer->drawText("Press R to restart", 0, sh * 0.65f, 0.8f, glm::vec4(0.8f, 0.8f, 0.8f, 1.0f), true);
    }
}

void GameState::generateFood() {
    // Modern C++ Hardware Random Generation (much better than standard rand()!)
    static std::random_device rd;  // Obtains a true random number from the hardware if available
    static std::mt19937 gen(rd()); // Seeds the Mersenne Twister engine with it
    std::uniform_int_distribution<int> disX(0, m_gridWidth - 1);  // Creates an exact distribution range
    std::uniform_int_distribution<int> disY(0, m_gridHeight - 1);
    
    // Do-While loop: Roll a random X/Y coordinate. If that coordinate lands perfectly on top 
    // of a piece of the snake's body, roll the dice again!
    do {
        m_foodPosition.x = static_cast<float>(disX(gen));
        m_foodPosition.y = static_cast<float>(disY(gen));
    } while (m_snake.checkCollision(m_foodPosition));
}

bool GameState::checkWallCollision(const glm::vec2& position) {
    // If the snake steps left of 0, right of the max width, above 0, or below max height, it's dead.
    return position.x < 0 || position.x >= m_gridWidth ||
           position.y < 0 || position.y >= m_gridHeight;
}
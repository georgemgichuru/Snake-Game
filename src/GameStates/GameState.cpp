#include "GameStates/GameState.h"
#include <iostream>
#include <random>

// Returns the correct texture name for each body segment based on its neighbors.
// toHead / toTail are the grid-directions FROM this segment TOWARD its neighbors.
static Direction directionBetween(const glm::vec2& from, const glm::vec2& to) {
    glm::vec2 d = to - from;
    if      (d.x >  0) return Direction::RIGHT;
    else if (d.x <  0) return Direction::LEFT;
    else if (d.y >  0) return Direction::DOWN;
    else               return Direction::UP;
}

static std::string headSprite(Direction dir) {
    switch (dir) {
        case Direction::UP:    return "head_up";
        case Direction::DOWN:  return "head_down";
        case Direction::LEFT:  return "head_left";
        case Direction::RIGHT: return "head_right";
    }
    return "head_right";
}

static std::string tailSprite(const glm::vec2& tail, const glm::vec2& beforeTail) {
    // The tail "points away" from the body — opposite of direction toward beforeTail
    Direction toward = directionBetween(tail, beforeTail);
    switch (toward) {
        case Direction::UP:    return "tail_down";
        case Direction::DOWN:  return "tail_up";
        case Direction::LEFT:  return "tail_right";
        case Direction::RIGHT: return "tail_left";
    }
    return "tail_right";
}

static std::string bodySprite(const glm::vec2& prev,   // head-side neighbor
                               const glm::vec2& curr,
                               const glm::vec2& next) { // tail-side neighbor
    Direction toHead = directionBetween(curr, prev);
    Direction toTail = directionBetween(curr, next);

    // Straight pieces
    bool horizontal = (toHead == Direction::LEFT  || toHead == Direction::RIGHT) &&
                      (toTail == Direction::LEFT  || toTail == Direction::RIGHT);
    bool vertical   = (toHead == Direction::UP    || toHead == Direction::DOWN)  &&
                      (toTail == Direction::UP    || toTail == Direction::DOWN);

    if (horizontal) return "body_horizontal";
    if (vertical)   return "body_vertical";

    // Corner pieces — named by which corner the curve passes through
    //   body_topleft     connects LEFT  + UP
    //   body_topright    connects RIGHT + UP
    //   body_bottomleft  connects LEFT  + DOWN
    //   body_bottomright connects RIGHT + DOWN
    auto has = [&](Direction a, Direction b) {
        return (toHead == a && toTail == b) || (toHead == b && toTail == a);
    };
    if (has(Direction::LEFT,  Direction::UP))   return "body_topleft";
    if (has(Direction::RIGHT, Direction::UP))   return "body_topright";
    if (has(Direction::LEFT,  Direction::DOWN)) return "body_bottomleft";
    if (has(Direction::RIGHT, Direction::DOWN)) return "body_bottomright";

    return "body_horizontal"; // fallback
}

GameState::GameState(Renderer* renderer, InputManager* input)
    : m_renderer(renderer)
    , m_input(input)
    , m_particles(500)
    , m_gridWidth(20)
    , m_gridHeight(15)
    , m_cellSize(40.0f)
    , m_isGameOver(false)
    , m_isPaused(false)
    , m_score(0) {
    
    // Calculate grid offset to center the game
    float totalWidth = m_gridWidth * m_cellSize;
    float totalHeight = m_gridHeight * m_cellSize;
    m_gridOffset = glm::vec2(
        (m_renderer->getScreenWidth() - totalWidth) / 2.0f,
        (m_renderer->getScreenHeight() - totalHeight) / 2.0f
    );
    
    m_foodColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red food
}

void GameState::enter() {
    m_soloud.init();
    m_eatSound.load("res/assets/untitled.wav");
    m_dieSound.load("res/assets/untitled2.wav");

    m_appState = AppState::START;

    Position startPos(m_gridWidth / 2, m_gridHeight / 2);
    m_snake.init(startPos, 3);
    m_isGameOver = false;
    m_isPaused   = false;
    m_score      = 0;
    m_highScore  = 0;
    generateFood();

    // Load all snake + food sprites
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

    // Init text rendering with a font from res/assets/fonts/
    m_renderer->initText("res/assets/fonts/font.ttf", 28);
}

void GameState::exit() {
    m_soloud.deinit();
}

void GameState::resetGame() {
    Position startPos(m_gridWidth / 2, m_gridHeight / 2);
    m_snake.init(startPos, 3);
    m_isGameOver = false;
    m_isPaused   = false;
    m_score      = 0;
    generateFood();
    // Reset particle timer so there's no burst of old particles
    static float movementTimer = 0.0f;
    movementTimer = 0.0f;
}

void GameState::update(float dt) {
    switch (m_appState) {

    case AppState::START:
        if (m_input->isKeyJustPressed(GLFW_KEY_ENTER)) {
            resetGame();
            m_appState = AppState::PLAYING;
        }
        break;

    case AppState::GAME_OVER:
        if (m_input->isKeyJustPressed(GLFW_KEY_R)) {
            resetGame();
            m_appState = AppState::PLAYING;
        }
        if (m_input->isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            // Let the main loop handle window close via ESCAPE
        }
        break;

    case AppState::PAUSED:
        if (m_input->isKeyJustPressed(GLFW_KEY_P)) {
            m_appState  = AppState::PLAYING;
            m_isPaused  = false;
        }
        break;

    case AppState::PLAYING: {
        // Cap dt to 100ms max — prevents a huge first-frame spike
        // from making the snake jump multiple cells instantly
        if (dt > 0.1f) dt = 0.1f;

        // Direction input
        if (m_input->isKeyJustPressed(GLFW_KEY_UP))    m_snake.setDirection(Direction::UP);
        else if (m_input->isKeyJustPressed(GLFW_KEY_DOWN))  m_snake.setDirection(Direction::DOWN);
        else if (m_input->isKeyJustPressed(GLFW_KEY_LEFT))  m_snake.setDirection(Direction::LEFT);
        else if (m_input->isKeyJustPressed(GLFW_KEY_RIGHT)) m_snake.setDirection(Direction::RIGHT);

        if (m_input->isKeyJustPressed(GLFW_KEY_P)) {
            m_appState = AppState::PAUSED;
            m_isPaused = true;
            break;
        }

        static float movementTimer = 0.0f;
        movementTimer += dt;
        const float MOVE_INTERVAL = 0.15f;

        while (movementTimer >= MOVE_INTERVAL) {
            m_snake.move();
            movementTimer -= MOVE_INTERVAL;

            glm::vec2 headScreenPos = m_gridOffset +
                glm::vec2(m_snake.getHead().x * m_cellSize + m_cellSize / 2.0f,
                          m_snake.getHead().y * m_cellSize + m_cellSize / 2.0f);
            m_particles.Emit(1, headScreenPos, glm::vec2(0.0f), glm::vec4(0.2f, 1.0f, 0.2f, 0.5f));

            // Food collision
            if (m_snake.getHead().x == m_foodPosition.x &&
                m_snake.getHead().y == m_foodPosition.y) {
                m_soloud.play(m_eatSound);
                glm::vec2 foodWorldPos = m_gridOffset +
                    glm::vec2(m_foodPosition.x * m_cellSize + m_cellSize / 2.0f,
                              m_foodPosition.y * m_cellSize + m_cellSize / 2.0f);
                m_particles.Emit(20, foodWorldPos, glm::vec2(0.0f), m_foodColor);
                m_snake.grow();
                m_score++;
                if (m_score > m_highScore) m_highScore = m_score;
                generateFood();
            }

            // Death collision
            if (checkWallCollision(m_snake.getHead()) || m_snake.checkSelfCollision()) {
                m_isGameOver = true;
                m_appState   = AppState::GAME_OVER;
                m_soloud.play(m_dieSound);
                if (m_gameOverCallback) m_gameOverCallback();
                break;
            }
        }

        m_particles.Update(dt);
        break;
    } // case PLAYING

    } // switch
}

void GameState::render() {
    // ── Always draw the playfield so it shows through all overlays ──
    m_renderer->drawRect(
        m_gridOffset,
        glm::vec2(m_gridWidth * m_cellSize, m_gridHeight * m_cellSize),
        glm::vec4(0.12f, 0.12f, 0.15f, 1.0f)
    );

    // Grid lines
    for (int i = 0; i <= m_gridWidth; i++) {
        m_renderer->drawRect(
            glm::vec2(m_gridOffset.x + i * m_cellSize, m_gridOffset.y),
            glm::vec2(1.0f, m_gridHeight * m_cellSize),
            glm::vec4(0.2f, 0.2f, 0.25f, 0.4f)
        );
    }
    for (int i = 0; i <= m_gridHeight; i++) {
        m_renderer->drawRect(
            glm::vec2(m_gridOffset.x, m_gridOffset.y + i * m_cellSize),
            glm::vec2(m_gridWidth * m_cellSize, 1.0f),
            glm::vec4(0.2f, 0.2f, 0.25f, 0.4f)
        );
    }

    // ── Snake + Food (show even on START so the player sees the board) ──
    const auto& segments = m_snake.getSegments();
    auto* rm = ResourceManager::getInstance();

    for (size_t i = 0; i < segments.size(); i++) {
        glm::vec2 pos(m_gridOffset + glm::vec2(segments[i].x * m_cellSize,
                                                segments[i].y * m_cellSize));
        glm::vec2 sz(m_cellSize, m_cellSize);
        std::string spriteName;
        if (i == 0)
            spriteName = headSprite(m_snake.getDirection());
        else if (i == segments.size() - 1)
            spriteName = tailSprite(segments[i], segments[i - 1]);
        else
            spriteName = bodySprite(segments[i - 1], segments[i], segments[i + 1]);

        auto tex = rm->getTexture(spriteName);
        if (tex)
            m_renderer->drawSprite(tex, pos, sz);
        else {
            glm::vec2 ip = pos + glm::vec2(2.0f);
            glm::vec2 is = glm::vec2(m_cellSize - 4.0f);
            m_renderer->drawRect(ip, is, i == 0
                ? glm::vec4(0.2f, 1.0f, 0.2f, 1.0f)
                : glm::vec4(0.1f, 0.6f, 0.1f, 1.0f));
        }
    }

    glm::vec2 foodPos = m_gridOffset +
        glm::vec2(m_foodPosition.x * m_cellSize, m_foodPosition.y * m_cellSize);
    auto appleTex = rm->getTexture("apple");
    if (appleTex)
        m_renderer->drawSprite(appleTex, foodPos, glm::vec2(m_cellSize));
    else
        m_renderer->drawRect(foodPos + glm::vec2(4.0f),
                             glm::vec2(m_cellSize - 8.0f), m_foodColor);

    m_particles.Draw(m_renderer);

    // ── Shared score strip at the top ──
    float sw = static_cast<float>(m_renderer->getScreenWidth());

    if (m_appState == AppState::PLAYING || m_appState == AppState::PAUSED ||
        m_appState == AppState::GAME_OVER) {
        m_renderer->drawText("SCORE  " + std::to_string(m_score),
                             20.0f, 10.0f, 0.8f,
                             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        m_renderer->drawText("BEST  " + std::to_string(m_highScore),
                             sw - 180.0f, 10.0f, 0.8f,
                             glm::vec4(0.8f, 0.8f, 0.2f, 1.0f));
    }

    // ── State-specific overlays ──
    float sh = static_cast<float>(m_renderer->getScreenHeight());

    if (m_appState == AppState::START) {
        // Dark vignette so text pops
        m_renderer->drawRect({0, 0}, {sw, sh}, {0.0f, 0.0f, 0.0f, 0.55f});

        m_renderer->drawText("SNAKE",
                             0, sh * 0.30f, 2.0f,
                             glm::vec4(0.2f, 1.0f, 0.2f, 1.0f),
                             /*centreX=*/true);
        m_renderer->drawText("Press ENTER to Start",
                             0, sh * 0.50f, 0.9f,
                             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                             true);
        m_renderer->drawText("Arrow keys to move   P to pause",
                             0, sh * 0.62f, 0.65f,
                             glm::vec4(0.7f, 0.7f, 0.7f, 1.0f),
                             true);
    }

    if (m_appState == AppState::PAUSED) {
        m_renderer->drawRect({0, 0}, {sw, sh}, {0.0f, 0.0f, 0.0f, 0.5f});

        m_renderer->drawText("PAUSED",
                             0, sh * 0.40f, 1.6f,
                             glm::vec4(1.0f, 1.0f, 0.2f, 1.0f),
                             true);
        m_renderer->drawText("Press P to continue",
                             0, sh * 0.57f, 0.85f,
                             glm::vec4(0.9f, 0.9f, 0.9f, 1.0f),
                             true);
    }

    if (m_appState == AppState::GAME_OVER) {
        m_renderer->drawRect({0, 0}, {sw, sh}, {0.5f, 0.0f, 0.0f, 0.55f});

        m_renderer->drawText("GAME OVER",
                             0, sh * 0.35f, 1.6f,
                             glm::vec4(1.0f, 0.2f, 0.2f, 1.0f),
                             true);
        m_renderer->drawText("Score: " + std::to_string(m_score),
                             0, sh * 0.52f, 1.0f,
                             glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
                             true);
        m_renderer->drawText("Press R to restart",
                             0, sh * 0.65f, 0.8f,
                             glm::vec4(0.8f, 0.8f, 0.8f, 1.0f),
                             true);
    }
}

void GameState::generateFood() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<int> disX(0, m_gridWidth - 1);
    std::uniform_int_distribution<int> disY(0, m_gridHeight - 1);
    
    // Generate food at random position not occupied by snake
    do {
        m_foodPosition.x = static_cast<float>(disX(gen));
        m_foodPosition.y = static_cast<float>(disY(gen));
    } while (m_snake.checkCollision(m_foodPosition));
}

bool GameState::checkWallCollision(const glm::vec2& position) {
    return position.x < 0 || position.x >= m_gridWidth ||
           position.y < 0 || position.y >= m_gridHeight;
}
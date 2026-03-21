#pragma once
#include <string>
#include <map>
#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

// One cached entry per ASCII glyph
struct Character {
    unsigned int textureID; // Single-channel GPU texture of this glyph bitmap
    glm::ivec2   size;      // Pixel dimensions of the glyph
    glm::ivec2   bearing;   // Offset from baseline to top-left corner
    unsigned int advance;   // How far right to move for the next character (1/64 px units)
};

class TextRenderer {
public:
    TextRenderer(int screenWidth, int screenHeight);
    ~TextRenderer();

    // Call once after OpenGL is ready. fontPath = path to a .ttf file.
    bool init(const std::string& fontPath, unsigned int fontSize = 24);

    // Measures how wide (pixels) a string will be at the given scale
    float measureText(const std::string& text, float scale) const;

    void drawText(const std::string& text,
                  float x, float y,
                  float scale,
                  const glm::vec4& color);

private:
    std::map<char, Character> m_characters;
    unsigned int m_vao           = 0;
    unsigned int m_vbo           = 0;
    unsigned int m_shaderProgram = 0;
    glm::mat4    m_projection;

    // Cached uniform locations — resolved once in init(), reused every draw
    int m_locTextColor  = -1;
    int m_locText       = -1;
    int m_locProjection = -1;

    unsigned int compileShader(const char* src, unsigned int type);
    unsigned int linkShaderProgram(unsigned int vert, unsigned int frag);
    std::string  readFile(const std::string& path);
};
#include "Core/TextRenderer.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <fstream>
#include <sstream>
#include <iostream>
#include "glm/gtc/type_ptr.hpp"

TextRenderer::TextRenderer(int screenWidth, int screenHeight) {
    // Top-left origin to match the rest of the renderer
    m_projection = glm::ortho(0.0f, static_cast<float>(screenWidth),
                              static_cast<float>(screenHeight), 0.0f);
}

TextRenderer::~TextRenderer() {
    // Free every cached glyph texture
    for (auto& [ch, glyph] : m_characters)
        glDeleteTextures(1, &glyph.textureID);

    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_shaderProgram) glDeleteProgram(m_shaderProgram);
}

bool TextRenderer::init(const std::string& fontPath, unsigned int fontSize) {
    // ----------------------------------------------------------------
    // 1. Compile the dedicated text shaders
    // ----------------------------------------------------------------
    std::string vertSrc = readFile("shaders/text_vertex.glsl");
    std::string fragSrc = readFile("shaders/text_fragment.glsl");

    if (vertSrc.empty() || fragSrc.empty()) {
        std::cerr << "[TextRenderer] Failed to read text shaders\n";
        return false;
    }

    unsigned int vert = compileShader(vertSrc.c_str(), GL_VERTEX_SHADER);
    unsigned int frag = compileShader(fragSrc.c_str(), GL_FRAGMENT_SHADER);
    if (!vert || !frag) return false;

    m_shaderProgram = linkShaderProgram(vert, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);
    if (!m_shaderProgram) return false;

    // After linkShaderProgram succeeds, cache the uniform locations once
    glUseProgram(m_shaderProgram);
    m_locProjection = glGetUniformLocation(m_shaderProgram, "projection");
    m_locTextColor  = glGetUniformLocation(m_shaderProgram, "textColor");
    m_locText       = glGetUniformLocation(m_shaderProgram, "text");

    // Upload projection once here — it never changes
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projection));

    // ----------------------------------------------------------------
    // 2. Initialise FreeType and load the font face
    // ----------------------------------------------------------------
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "[TextRenderer] Failed to init FreeType\n";
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "[TextRenderer] Failed to load font: " << fontPath << "\n";
        FT_Done_FreeType(ft);
        return false;
    }

    // 0 width = auto-calculate from height
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // ----------------------------------------------------------------
    // 3. Rasterise and cache every printable ASCII character (32–126)
    // ----------------------------------------------------------------
    // FreeType bitmaps are 1 byte per pixel — disable the default 4-byte alignment
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 32; c < 127; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "[TextRenderer] Failed to load glyph '" << c << "'\n";
            continue;
        }

        // Upload the bitmap as a single-channel RED texture
        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                     face->glyph->bitmap.width,
                     face->glyph->bitmap.rows,
                     0, GL_RED, GL_UNSIGNED_BYTE,
                     face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character ch{
            tex,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left,  face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x) // stored in 1/64 px
        };
        m_characters[static_cast<char>(c)] = ch;
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); // restore default
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // ----------------------------------------------------------------
    // 4. Create a VAO/VBO for streaming glyph quads
    //    Each quad = 6 vertices × 4 floats (x, y, u, v)
    //    GL_DYNAMIC_DRAW because we rewrite it every character
    // ----------------------------------------------------------------
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

float TextRenderer::measureText(const std::string& text, float scale) const {
    float width = 0.0f;
    for (char c : text) {
        auto it = m_characters.find(c);
        if (it == m_characters.end()) continue;
        // advance is in 1/64th pixel units — shift right by 6 to convert
        width += (it->second.advance >> 6) * scale;
    }
    return width;
}

void TextRenderer::drawText(const std::string& text,
                             float x, float y,
                             float scale,
                             const glm::vec4& color) {
    glUseProgram(m_shaderProgram);
    // Use cached locations — no per-frame GPU string lookup
    glUniform4f(m_locTextColor, color.r, color.g, color.b, color.a);
    glUniform1i(m_locText, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(m_vao);

    for (char c : text) {
        auto it = m_characters.find(c);
        if (it == m_characters.end()) continue;
        const Character& ch = it->second;

        // Top-left origin: bearing.y is up from baseline, so subtract it
        float xPos = x + ch.bearing.x * scale;
        float yPos = y + (m_characters.at('A').bearing.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // Build the quad for this glyph — two triangles, CCW winding
        // UV: (0,0) top-left to (1,1) bottom-right
        float verts[6][4] = {
            { xPos,     yPos,     0.0f, 0.0f },
            { xPos,     yPos + h, 0.0f, 1.0f },
            { xPos + w, yPos + h, 1.0f, 1.0f },

            { xPos,     yPos,     0.0f, 0.0f },
            { xPos + w, yPos + h, 1.0f, 1.0f },
            { xPos + w, yPos,     1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance cursor: stored as 1/64th pixels, shift right 6 bits = divide by 64
        x += (ch.advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// ----------------------------------------------------------------
// Private helpers
// ----------------------------------------------------------------
std::string TextRenderer::readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[TextRenderer] Cannot open: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

unsigned int TextRenderer::compileShader(const char* src, unsigned int type) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int ok;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(id, 512, nullptr, log);
        std::cerr << "[TextRenderer] Shader error: " << log << "\n";
        return 0;
    }
    return id;
}

unsigned int TextRenderer::linkShaderProgram(unsigned int vert, unsigned int frag) {
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    int ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        std::cerr << "[TextRenderer] Link error: " << log << "\n";
        return 0;
    }
    return prog;
}
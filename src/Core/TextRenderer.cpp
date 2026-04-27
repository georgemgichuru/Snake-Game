#include "Core/TextRenderer.h"
#include <ft2build.h> // Include FreeType master file
#include FT_FREETYPE_H // Macro resolving to FreeType's main header
#include <fstream>
#include <sstream>
#include <iostream>
#include "glm/gtc/type_ptr.hpp"

TextRenderer::TextRenderer(int screenWidth, int screenHeight) {
    // Creates a 2D camera matrix where (0,0) is the Top-Left of the screen, matching the sprite renderer.
    m_projection = glm::ortho(0.0f, static_cast<float>(screenWidth), static_cast<float>(screenHeight), 0.0f);
}

TextRenderer::~TextRenderer() {
    // Loop through our map and delete every single letter's texture from GPU memory
    for (auto& [ch, glyph] : m_characters) {
        glDeleteTextures(1, &glyph.textureID);
    }
    // Delete the quad buffers and shader program
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
    if (!vert || !frag) return false; // Safety check

    m_shaderProgram = linkShaderProgram(vert, frag);
    
    // We don't need the individual shader objects after they are linked into a program
    glDeleteShader(vert);
    glDeleteShader(frag);
    if (!m_shaderProgram) return false;

    // Cache the memory locations of the shader uniforms so we can easily pass data to them later
    glUseProgram(m_shaderProgram);
    m_locProjection = glGetUniformLocation(m_shaderProgram, "projection");
    m_locTextColor  = glGetUniformLocation(m_shaderProgram, "textColor");
    m_locText       = glGetUniformLocation(m_shaderProgram, "text");

    // The projection matrix never changes, so we can upload it to the shader just once right now.
    glUniformMatrix4fv(m_locProjection, 1, GL_FALSE, glm::value_ptr(m_projection));

    // ----------------------------------------------------------------
    // 2. Initialise FreeType and load the font face
    // ----------------------------------------------------------------
    FT_Library ft; // Create the main FreeType library object
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "[TextRenderer] Failed to init FreeType\n";
        return false;
    }

    FT_Face face; // Create a Face object which represents the specific font file (.ttf)
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "[TextRenderer] Failed to load font: " << fontPath << "\n";
        FT_Done_FreeType(ft); // Clean up on failure
        return false;
    }

    // Set the pixel size of the font. Passing '0' to width tells it to automatically scale width based on the height.
    FT_Set_Pixel_Sizes(face, 0, fontSize);

    // ----------------------------------------------------------------
    // 3. Rasterise and cache every printable ASCII character (32–126)
    // ----------------------------------------------------------------
    // MAGIC TRICK #1: OpenGL expects texture rows to be a multiple of 4 bytes. 
    // Text fonts are grayscale (1 byte per pixel). This forces OpenGL to accept 1-byte alignment so text doesn't look distorted!
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Loop through the standard ASCII table (32 is 'Space', 126 is '~')
    for (unsigned char c = 32; c < 127; c++) {
        // Ask FreeType to mathematically draw the character into a bitmap image
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cerr << "[TextRenderer] Failed to load glyph '" << c << "'\n";
            continue; // If it fails, just skip to the next letter
        }

        // Generate a new OpenGL Texture
        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        
        // Upload the FreeType bitmap to the GPU as a GL_RED texture (only 1 color channel instead of RGB)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

        // Set texture wrap and filter options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Create our Character struct, saving the texture ID and all the physical spacing metrics
        Character ch = {
            tex,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left,  face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x) // MAGIC TRICK #2: Advance is stored in 1/64th of a pixel!
        };
        // Add the character to our dictionary
        m_characters[static_cast<char>(c)] = ch;
    }

    // Reset OpenGL's unpack alignment back to standard 4-byte so we don't break image/sprite loading elsewhere
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4); 
    
    // Clean up FreeType memory since we are done extracting the images
    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    // ----------------------------------------------------------------
    // 4. Create a VAO/VBO for streaming glyph quads
    // ----------------------------------------------------------------
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    
    // Allocate exactly enough memory for ONE quad (6 vertices * 4 floats [x,y,u,v]).
    // We use GL_DYNAMIC_DRAW because we will overwrite this buffer for every single letter we draw!
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, nullptr, GL_DYNAMIC_DRAW);

    // Tell OpenGL how to read the buffer: Location 0, 4 floats per vertex, no normalization, 4 floats total stride, starting at 0.
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    // Unbind to keep things safe
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return true;
}

float TextRenderer::measureText(const std::string& text, float scale) const {
    float width = 0.0f;
    for (char c : text) { // Loop through every letter in the string
        auto it = m_characters.find(c); // Find the letter in our dictionary
        if (it == m_characters.end()) continue; // If not found, skip
        
        // Bitshift right by 6 (>> 6) is the math equivalent of dividing by 64!
        // We do this to convert FreeType's 1/64th pixel units into standard pixels.
        width += (it->second.advance >> 6) * scale; 
    }
    return width; // Return the total horizontal width in pixels
}

void TextRenderer::drawText(const std::string& text, float x, float y, float scale, const glm::vec4& color) {
    glUseProgram(m_shaderProgram); // Activate text shader
    
    // Send the requested font color to the fragment shader
    glUniform4f(m_locTextColor, color.r, color.g, color.b, color.a);
    glUniform1i(m_locText, 0); // Tell it to look at Texture Slot 0

    glActiveTexture(GL_TEXTURE0); // Activate Texture Slot 0
    glBindVertexArray(m_vao); // Bind our empty text Quad

    // Loop through every character in the string the user wants to draw
    for (char c : text) {
        auto it = m_characters.find(c); // Look it up in our dictionary
        if (it == m_characters.end()) continue;
        const Character& ch = it->second;

        // MAGIC TRICK #3: Baseline Math. 
        // Not all letters sit flat. A 'p' sits on the baseline, but a 'y' dips below it.
        float xPos = x + ch.bearing.x * scale;
        
        // We find the tallest letter ('A'), and subtract the current letter's bearing to push smaller/dipping letters down visually.
        float yPos = y + (m_characters.at('A').bearing.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale; // Calculate scaled width
        float h = ch.size.y * scale; // Calculate scaled height

        // Overwrite our 2D array with the exact exact screen coordinates (xPos, yPos) 
        // and texture coordinates (0 to 1) for this specific letter.
        float verts[6][4] = {
            { xPos,     yPos,     0.0f, 0.0f }, // Top-Left
            { xPos,     yPos + h, 0.0f, 1.0f }, // Bottom-Left
            { xPos + w, yPos + h, 1.0f, 1.0f }, // Bottom-Right

            { xPos,     yPos,     0.0f, 0.0f }, // Top-Left
            { xPos + w, yPos + h, 1.0f, 1.0f }, // Bottom-Right
            { xPos + w, yPos,     1.0f, 0.0f }  // Top-Right
        };

        // Bind the specific texture for this letter (e.g., the image of the 'a')
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        
        // Push the newly calculated float array (verts) into the GPU VBO! (This is why it is GL_DYNAMIC_DRAW)
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts); 
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Tell OpenGL to draw the two triangles
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Advance the X coordinate cursor rightward so the next letter doesn't draw on top of this one.
        // Again, bitshift by 6 to divide by 64.
        x += (ch.advance >> 6) * scale;
    }

    // Clean up state
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// ----------------------------------------------------------------
// Private helpers
// ----------------------------------------------------------------
std::string TextRenderer::readFile(const std::string& path) {
    std::ifstream f(path); // Open the file
    if (!f.is_open()) {
        std::cerr << "[TextRenderer] Cannot open: " << path << "\n";
        return "";
    }
    std::stringstream ss;
    ss << f.rdbuf(); // Read the entire file buffer into a stringstream
    return ss.str(); // Convert to standard C++ string
}

unsigned int TextRenderer::compileShader(const char* src, unsigned int type) {
    unsigned int id = glCreateShader(type); // Ask GPU for an empty shader slot
    glShaderSource(id, 1, &src, nullptr); // Give the GPU the string source code
    glCompileShader(id); // Tell the GPU to compile it

    int ok;
    glGetShaderiv(id, GL_COMPILE_STATUS, &ok); // Check if compilation was successful
    if (!ok) {
        char log[512];
        glGetShaderInfoLog(id, 512, nullptr, log); // Get the exact GLSL syntax error
        std::cerr << "[TextRenderer] Shader error: " << log << "\n";
        return 0;
    }
    return id;
}

unsigned int TextRenderer::linkShaderProgram(unsigned int vert, unsigned int frag) {
    unsigned int prog = glCreateProgram(); // Ask GPU for an empty program slot
    glAttachShader(prog, vert); // Attach vertex shader
    glAttachShader(prog, frag); // Attach fragment shader
    glLinkProgram(prog); // Link them together

    int ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok); // Check if linking was successful
    if (!ok) {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log); // Get the exact link error
        std::cerr << "[TextRenderer] Link error: " << log << "\n";
        return 0;
    }
    return prog;
}
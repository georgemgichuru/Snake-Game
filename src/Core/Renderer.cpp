#include "Core/Renderer.h"
#include <iostream>

// Vertex data for a 1x1 Unit Quad (two triangles).
// Since the coordinates map exactly from 0.0 to 1.0, scaling this quad by a 
// vector like (50, 50) makes it exactly 50x50 pixels large on screen!
const float vertices[] = {
    // Position (x, y)    // Texture coord (u, v)
    0.0f, 0.0f,           0.0f, 0.0f, // Top-left corner
    1.0f, 0.0f,           1.0f, 0.0f, // Top-right corner
    1.0f, 1.0f,           1.0f, 1.0f, // Bottom-right corner
    0.0f, 1.0f,           0.0f, 1.0f  // Bottom-left corner
};

// Indices telling OpenGL which points connect to form the two triangles of our rectangle
const unsigned int indices[] = {
    0, 1, 2, // Triangle 1 (Top-Left, Top-Right, Bottom-Right)
    2, 3, 0  // Triangle 2 (Bottom-Right, Bottom-Left, Top-Left)
};

Renderer::Renderer(int width, int height) 
    : m_vao(0)
    , m_vbo(0)
    , m_ebo(0)
    , m_screenWidth(width)
    , m_screenHeight(height)
    // Orthographic projection: Creates a 2D camera. 
    // Left=0, Right=width, Bottom=height, Top=0. (Note: Y-axis is flipped so 0 is at the top of the screen)
    , m_projection(glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f)) {
}

Renderer::~Renderer() {
    // Clean up our GPU buffers when the game closes to prevent memory leaks
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

bool Renderer::init() {
    // Grab the resource manager to load our GLSL shader files
    auto resourceManager = ResourceManager::getInstance();
    m_shader = resourceManager->loadShader("basic", "shaders/vertex.glsl", "shaders/fragment.glsl");
    
    if (!m_shader) {
        std::cerr << "Failed to load shader!" << std::endl;
        return false; // Exit out if shaders fail
    }
    
    // Generate OpenGL buffers
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    
    // Bind the VAO first! All subsequent VBO/EBO setup will be memorized by this VAO.
    glBindVertexArray(m_vao);
    
    // Bind VBO and push our float array containing positions and texture coordinates
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Bind EBO and push our integer array containing the triangle drawing order
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Tell OpenGL how to read the positions from the float array
    // (Location 0, 2 floats, no normalization, skip 4 floats per row, start at byte 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Tell OpenGL how to read the Texture Coordinates (UVs) from the float array
    // (Location 1, 2 floats, no normalization, skip 4 floats per row, start after 2 floats)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Safely unbind the buffers now that the VAO has recorded them
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Enable Alpha Blending (Allows transparent PNGs and particles to work properly)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Set the default clear color (Very dark gray)
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f); 
    
    return true;
}

void Renderer::clear() const {
    // Wipes the screen using the color specified in glClearColor
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::beginFrame() {
    clear(); // Wipe the previous frame
    if (m_shader) {
        m_shader->use(); // Activate our shader
        m_shader->setMat4("projection", m_projection); // Send the 2D camera matrix to the shader
        m_shader->setInt("image", 0);       // Tell the shader the texture is in slot 0
        m_shader->setInt("useTexture", 0);  // By default, assume we are drawing solid colors, not textures
    }
}

void Renderer::endFrame() {
    // Left empty for now. In advanced games, you'd swap framebuffers or process post-processing effects here.
}

void Renderer::drawRect(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
    if (!m_shader) return;
    
    // GLM math applies right-to-left. So we SCALE the 1x1 quad first, then TRANSLATE it to the position.
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix
    model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f)); // Move it
    model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f)); // Resize it
    
    m_shader->setMat4("model", model); // Send the transform matrix to the shader
    m_shader->setVec4("color", color); // Send the requested color
    m_shader->setInt("useTexture", 0); // Ensure the fragment shader knows NOT to sample an image
    
    glBindVertexArray(m_vao); // Bind our 1x1 quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); // Draw the 6 vertices (2 triangles)
    glBindVertexArray(0); // Unbind
}

void Renderer::drawSprite(const std::shared_ptr<Texture2D>& texture, const glm::vec2& position, const glm::vec2& size, const glm::vec4& tint) {
    if (!m_shader || !texture) return; // Safety check

    // Same math logic as drawRect
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));
    model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));

    m_shader->setMat4("model", model);
    m_shader->setVec4("color", tint); // Usually white (1,1,1,1) so the texture looks normal
    m_shader->setInt("useTexture", 1); // Tell the fragment shader to use the image!

    texture->bind(0); // Bind the texture memory to GPU slot 0

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Reset back to colored rect mode to prevent bugs on the next drawRect call
    m_shader->setInt("useTexture", 0); 
}

bool Renderer::initText(const std::string& fontPath, unsigned int fontSize) {
    // Instantiates our FreeType text renderer
    m_textRenderer = std::make_unique<TextRenderer>(m_screenWidth, m_screenHeight);
    return m_textRenderer->init(fontPath, fontSize);
}

void Renderer::drawText(const std::string& text, float x, float y, float scale, const glm::vec4& color, bool centreX) {
    if (!m_textRenderer) return; // Safety check

    // If centreX is true, calculate the exact width of the string in pixels, 
    // subtract it from the screen width, and divide by 2 to find the exact horizontal middle.
    if (centreX) {
        float w = m_textRenderer->measureText(text, scale);
        x = (m_screenWidth - w) / 2.0f;
    }

    m_textRenderer->drawText(text, x, y, scale, color);

    // The TextRenderer uses its own specific shader. 
    // We must re-bind our main sprite shader so that subsequent snake/apple draw calls don't break!
    if (m_shader) {
        m_shader->use();
        m_shader->setMat4("projection", m_projection);
        m_shader->setInt("image", 0);
        m_shader->setInt("useTexture", 0);
    }
}

void Renderer::setViewport(int width, int height) {
    m_screenWidth = width;
    m_screenHeight = height;
    
    // Tell OpenGL the new window dimensions
    glViewport(0, 0, width, height);
    
    // Recalculate our camera matrix so shapes don't get stretched/squished when the window changes shape
    m_projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
}
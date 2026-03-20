#include "Core/Renderer.h"
#include <iostream>

// Vertex data for a quad (two triangles) - Origins at top-left corner
const float vertices[] = {
    // Position (x, y)    // Texture coord (u, v)
    0.0f, 0.0f, 0.0f, 0.0f, // Top-left
    1.0f, 0.0f, 1.0f, 0.0f, // Top-right
    1.0f, 1.0f, 1.0f, 1.0f, // Bottom-right
    0.0f, 1.0f, 0.0f, 1.0f  // Bottom-left
};

const unsigned int indices[] = {
    0, 1, 2,
    2, 3, 0
};

Renderer::Renderer(int width, int height) 
    : m_vao(0)
    , m_vbo(0)
    , m_ebo(0)
    , m_screenWidth(width)
    , m_screenHeight(height)
    , m_projection(glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f)) {
}

Renderer::~Renderer() {
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);
}

bool Renderer::init() {
    // Load shader
    auto resourceManager = ResourceManager::getInstance();
    m_shader = resourceManager->loadShader("basic", "shaders/vertex.glsl", "shaders/fragment.glsl");
    
    if (!m_shader) {
        std::cerr << "Failed to load shader!" << std::endl;
        return false;
    }
    
    // Create VAO, VBO, EBO
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);
    
    glBindVertexArray(m_vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coord attribute (we'll use for color interpolation)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Enable blending for transparency and set default background clear color
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f); // Very dark gray by default
    
    return true;
}

void Renderer::clear() const {
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::beginFrame() {
    clear();
    if (m_shader) {
        m_shader->use();
        m_shader->setMat4("projection", m_projection);
    }
}

void Renderer::endFrame() {
    // Nothing needed here typically
}

void Renderer::drawRect(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
    if (!m_shader) return;
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position.x, position.y, 0.0f));
    model = glm::scale(model, glm::vec3(size.x, size.y, 1.0f));
    
    m_shader->setMat4("model", model);
    m_shader->setVec4("color", color);
    
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::drawText(const std::string& text, const glm::vec2& position, float scale, const glm::vec4& color) {
    // Simplified text rendering - in a real game, you'd use a font renderer
    // For now, we'll just use a simple placeholder
    (void)text;
    (void)position;
    (void)scale;
    (void)color;
}

void Renderer::setViewport(int width, int height) {
    m_screenWidth = width;
    m_screenHeight = height;
    glViewport(0, 0, width, height);
    m_projection = glm::ortho(0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
}
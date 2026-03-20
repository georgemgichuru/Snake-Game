#pragma once

#include <memory>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ResourceManager.h"

/**
 * Renderer - Handles all rendering operations
 */
class Renderer {
private:
    std::shared_ptr<Shader> m_shader;
    GLuint m_vao, m_vbo, m_ebo;
    glm::mat4 m_projection;
    int m_screenWidth;
    int m_screenHeight;
    
public:
    Renderer(int width, int height);
    ~Renderer();
    
    // Initialize OpenGL resources
    bool init();
    
    // Clear the screen
    void clear() const;
    
    // Begin rendering frame
    void beginFrame();
    
    // End rendering frame
    void endFrame();
    
    // Draw a rectangle at specified position with size and color
    void drawRect(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    
    // Draw text (simplified - you can expand this)
    void drawText(const std::string& text, const glm::vec2& position, float scale, const glm::vec4& color);
    
    // Set viewport size
    void setViewport(int width, int height);
    
    // Get screen dimensions
    int getScreenWidth() const { return m_screenWidth; }
    int getScreenHeight() const { return m_screenHeight; }
    
    // Get projection matrix
    const glm::mat4& getProjection() const { return m_projection; }
};
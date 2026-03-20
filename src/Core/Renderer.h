#pragma once

#include <memory>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "ResourceManager.h"

/**
 * Renderer Class:
 * Handles the lowest level of drawing graphics to the screen utilizing OpenGL.
 * Instead of making complex OpenGL API calls all over the codebase, we bundle
 * them up safely here so that the main game code just says "draw a rectangle".
 */
class Renderer {
private:
    std::shared_ptr<Shader> m_shader;
    
    // OpenGL hardware buffer IDs
    GLuint m_vao; // Vertex Array Object: Remembers buffer configurations
    GLuint m_vbo; // Vertex Buffer Object: Stores our raw polygon point data on the GPU
    GLuint m_ebo; // Element Buffer Object: Stores indices that tell the GPU how to connect the points
    
    // Viewport data
    glm::mat4 m_projection; // Matrix converting flat 2D game coords to proper screen pixels
    int m_screenWidth;
    int m_screenHeight;
    
public:
    Renderer(int width, int height);
    ~Renderer();
    
    // --------------------------------------------------------
    // SETUP & LIFECYCLE
    // --------------------------------------------------------
    
    // Compiles the shaders and prepares the memory buffers for drawing quads
    bool init();
    
    // Wipes the screen background clear to prevent trailing visual smudges
    void clear() const;
    
    // Prepares the hardware states needed BEFORE sending any draw commands
    void beginFrame();
    
    // Called when drawing is finished. Doesn't do much in 2D, but useful for 3D/Effects
    void endFrame();
    
    // --------------------------------------------------------
    // DRAWING UTILITIES
    // --------------------------------------------------------
    
    // Renders a colored rectangle at the targeted XY position with the given WidthxHeight sizes
    void drawRect(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
    
    // Placeholder for drawing fonts.
    void drawText(const std::string& text, const glm::vec2& position, float scale, const glm::vec4& color);
    
    // Update drawing logic if the native Window resizes
    void setViewport(int width, int height);
    
    // Get screen dimensions globally
    int getScreenWidth() const { return m_screenWidth; }
    int getScreenHeight() const { return m_screenHeight; }
    
    const glm::mat4& getProjection() const { return m_projection; }
};
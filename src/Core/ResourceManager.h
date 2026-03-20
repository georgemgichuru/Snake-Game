#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
 * Shader Object Class:
 * Represents an active piece of code sitting on the graphics card (GPU). 
 * Shaders are what actually do the math to draw pixels onto the monitor.
 * This class gives us simple C++ functions to plug data neatly into the GPU code.
 */
class Shader {
public:
    unsigned int ID; // The numeric ID given to the Shader natively by OpenGL
    
    Shader() : ID(0) {}
    ~Shader() { if (ID) glDeleteProgram(ID); } // Delete off the GPU to save memory
    
    // Activate this shader. Required before you can begin feeding it variables or drawing.
    void use() { glUseProgram(ID); }
    
    // --- UTILITY METHODS ---
    // The following "set" variables are used to inject data straight from our C++ CPU side
    // right into our GLSL shader files over on the GPU hardware side.
    
    void setBool(const std::string &name, bool value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    
    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    
    void setVec2(const std::string &name, const glm::vec2 &value) const {
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    
    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    
    void setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        // We often set "Matrix 4x4s" (mat4) when updating positions to adjust the 2D plane perspective
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
};

/**
 * ResourceManager Class:
 * A global tool responsible for safely loading files from your hard drive into Memory 
 * so that you don't load the same files (like massive textures or shaders) repeatedly.
 * Operates as a Singleton.
 */
class ResourceManager {
private:
    static ResourceManager* s_instance;
    // Map tracking our shaders by a convenient string name, like "basic"
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    
    ResourceManager() = default;
    ~ResourceManager() = default;
    
    // Internal parser to take text and turn it into GPU-readable shader code
    unsigned int compileShader(const char* source, GLenum type);
    // Mashes the Vertex shader and Fragment shader into one cohesive "Program"
    unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    
public:
    // Delete copy constructor & assignment to maintain singleton architecture
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    static ResourceManager* getInstance();
    
    // Opens up text files from the hard drive, parses the GLSL text, builds a shader,
    // and places it securely in our `m_shaders` tracking map for future use.
    std::shared_ptr<Shader> loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath);
    
    // Fetches an already-loaded shader by name from cache
    std::shared_ptr<Shader> getShader(const std::string& name);
    
    // General utility just to read any text file outright into a single string
    static std::string readFile(const std::string& path);
};
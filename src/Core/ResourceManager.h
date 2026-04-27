#pragma once

#include <unordered_map> // C++ Hash Map for ultra-fast O(1) lookups
#include <memory>        // For smart pointers (std::shared_ptr)
#include <string>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp> // Allows us to pass GLM math objects directly to OpenGL
#include "Texture2D.h"

/**
 * Shader Object Class:
 * A lightweight C++ wrapper around an OpenGL Shader Program.
 * It holds the GPU ID and provides helper functions to send C++ variables
 * (like matrices, colors, or floats) directly into the GLSL shader code running on the GPU.
 */
class Shader {
public:
    unsigned int ID; // The numeric ID OpenGL uses to identify this shader program natively.
    
    // Constructor initializes ID to 0 (which means "no shader" in OpenGL).
    Shader() : ID(0) {} 
    
    // Destructor securely deletes the program from GPU memory when this C++ object is destroyed.
    ~Shader() { if (ID) glDeleteProgram(ID); } 
    
    // Tells the GPU to actively use this specific shader program for all following draw calls.
    void use() { glUseProgram(ID); }
    
    // ------------------------------------------------------------------------
    // UNIFORM SETTERS: 
    // These find a variable by 'name' in your .glsl file and update its value.
    // ------------------------------------------------------------------------
    
    void setBool(const std::string &name, bool value) const {
        // glGetUniformLocation asks the GPU where the variable is. glUniform1i sets it (booleans are treated as 0 or 1).
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    
    void setInt(const std::string &name, int value) const {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
    }
    
    void setFloat(const std::string &name, float value) const {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
    
    void setVec2(const std::string &name, const glm::vec2 &value) const {
        // "2fv" means "2 floats, passed via a vector/array pointer".
        glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    
    void setVec3(const std::string &name, const glm::vec3 &value) const {
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    
    void setVec4(const std::string &name, const glm::vec4 &value) const {
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
    }
    
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        // glm::value_ptr translates the GLM matrix struct into a raw float array that C-based OpenGL can read.
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
};

/**
 * ResourceManager Class (Singleton):
 * Prevents loading the same file from the hard drive multiple times.
 * If 100 apples spawn, we only load "apple.png" once, and all 100 apples share it.
 */
class ResourceManager {
private:
    static ResourceManager* s_instance; // The single global instance of this manager.
    
    // Hash maps acting as our caches. Strings (names) map to Shared Pointers of the resources.
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    std::unordered_map<std::string, std::shared_ptr<Texture2D>> m_textures;
    
    ResourceManager() = default;  // Private constructor prevents anyone else from calling 'new ResourceManager()'
    ~ResourceManager() = default; 
    
    // Internal helper functions to compile and link raw GLSL text
    unsigned int compileShader(const char* source, GLenum type);
    unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    
public:
    // Delete copy/assignment operations so our Singleton can absolutely never be duplicated
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    static ResourceManager* getInstance(); // Public access point to get the manager
    
    // Compiles a new shader from file paths and saves it into the hash map under 'name'
    std::shared_ptr<Shader> loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath);
    
    // Fetches an already loaded shader from the hash map
    std::shared_ptr<Shader> getShader(const std::string& name);
    
    // Utility to dump an entire text file into a C++ string
    static std::string readFile(const std::string& path);

    // Decodes an image file and saves it into the hash map under 'name'
    std::shared_ptr<Texture2D> loadTexture(const std::string& name, const std::string& path);
    
    // Fetches an already loaded texture from the hash map
    std::shared_ptr<Texture2D> getTexture(const std::string& name);
};
#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

/**
 * Simple Shader class to handle shader programs
 */
class Shader {
public:
    unsigned int ID;
    
    Shader() : ID(0) {}
    ~Shader() { if (ID) glDeleteProgram(ID); }
    
    void use() { glUseProgram(ID); }
    
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
    
    void setMat4(const std::string &name, const glm::mat4 &mat) const {
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }
};

/**
 * ResourceManager - Singleton pattern to manage shaders and other resources
 */
class ResourceManager {
private:
    static ResourceManager* s_instance;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
    
    ResourceManager() = default;
    ~ResourceManager() = default;
    
    // Helper function to compile shader
    unsigned int compileShader(const char* source, GLenum type);
    // Helper function to link shader program
    unsigned int linkProgram(unsigned int vertexShader, unsigned int fragmentShader);
    
public:
    // Delete copy constructor and assignment operator
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    
    static ResourceManager* getInstance();
    
    // Load a shader from files
    std::shared_ptr<Shader> loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath);
    
    // Get a loaded shader
    std::shared_ptr<Shader> getShader(const std::string& name);
    
    // Read file contents
    static std::string readFile(const std::string& path);
};
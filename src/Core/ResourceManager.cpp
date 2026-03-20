#include "Core/ResourceManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

ResourceManager* ResourceManager::s_instance = nullptr;

ResourceManager* ResourceManager::getInstance() {
    if (!s_instance) {
        s_instance = new ResourceManager();
    }
    return s_instance;
}

unsigned int ResourceManager::compileShader(const char* source, GLenum type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    // Check compilation
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    return shader;
}

unsigned int ResourceManager::linkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Check linking
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        return 0;
    }
    
    return program;
}

std::string ResourceManager::readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath) {
    // Read shader files
    std::string vertexCode = readFile(vertPath);
    std::string fragmentCode = readFile(fragPath);
    
    if (vertexCode.empty() || fragmentCode.empty()) {
        return nullptr;
    }
    
    // Compile shaders
    unsigned int vertexShader = compileShader(vertexCode.c_str(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentCode.c_str(), GL_FRAGMENT_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        return nullptr;
    }
    
    // Link program
    unsigned int program = linkProgram(vertexShader, fragmentShader);
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (program == 0) {
        return nullptr;
    }
    
    // Create shader object
    auto shader = std::make_shared<Shader>();
    shader->ID = program;
    
    // Store in map
    m_shaders[name] = shader;
    
    return shader;
}

std::shared_ptr<Shader> ResourceManager::getShader(const std::string& name) {
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }
    return nullptr;
}
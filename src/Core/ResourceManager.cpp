#include "Core/ResourceManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

ResourceManager* ResourceManager::s_instance = nullptr;

ResourceManager* ResourceManager::getInstance() {
    // Basic lazy-initialization logic to spin up the Manager if it doesn't already exist
    if (!s_instance) {
        s_instance = new ResourceManager();
    }
    return s_instance;
}

unsigned int ResourceManager::compileShader(const char* source, GLenum type) {
    // Generate a blank canvas handle over on the GPU
    unsigned int shader = glCreateShader(type);
    
    // Upload our raw string C++ text reading from the .glsl files to the GPU Shader memory 
    glShaderSource(shader, 1, &source, nullptr);
    
    // Actually compile that text into runnable GPU commands natively
    glCompileShader(shader);
    
    // Error checking - Important! If you write a typo in GLSL, this will report why the game crashed.
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
        return 0;
    }
    
    // Return back the resulting hardware ID so we can refer to it later
    return shader;
}

unsigned int ResourceManager::linkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    // Combining Vertex & Fragment shaders into one cohesive rendering "Program" pipeline
    unsigned int program = glCreateProgram();
    
    // Glue them together...
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    
    // ...and link them so that the outputs of the Vertex shader flow securely to the Fragment shader inputs
    glLinkProgram(program);
    
    // Verify the linking was structurally sound
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
    
    // Super fast method in pure C++ to dump an entire text file neatly into a std::string buffer.
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath) {
    // 1. Read files verbatim out as plain text
    std::string vertexCode = readFile(vertPath);
    std::string fragmentCode = readFile(fragPath);
    
    if (vertexCode.empty() || fragmentCode.empty()) {
        return nullptr; // File misnamed or doesn't exist
    }
    
    // 2. Perform raw compilation on both chunks of code
    unsigned int vertexShader = compileShader(vertexCode.c_str(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentCode.c_str(), GL_FRAGMENT_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        return nullptr; // One of the shaders was corrupted
    }
    
    // 3. Bake both successfully compiled components into a single Program ID
    unsigned int program = linkProgram(vertexShader, fragmentShader);
    
    // Clean up original fragments to preserve memory; we only need the Linked Program going forward.
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (program == 0) {
        return nullptr;
    }
    
    // Generate a neat heap-allocated C++ object handle wrapping our GPU shader
    auto shader = std::make_shared<Shader>();
    shader->ID = program;
    
    // Cache it to easily fetch later through "m_shaders"
    m_shaders[name] = shader;
    
    return shader;
}

std::shared_ptr<Shader> ResourceManager::getShader(const std::string& name) {
    // Queries if we already loaded it
    auto it = m_shaders.find(name);
    if (it != m_shaders.end()) {
        return it->second;
    }
    return nullptr;
}
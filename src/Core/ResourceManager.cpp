// MAGIC EXPLANATION: stb_image is a "single-file header" library. 
// Defining this macro BEFORE including it tells the compiler to literally unpack 
// all the C code for image loading directly into this exact .cpp file!
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Core/ResourceManager.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Initialize our static singleton pointer to null before the program actually starts
ResourceManager* ResourceManager::s_instance = nullptr; 

ResourceManager* ResourceManager::getInstance() {
    if (!s_instance) { // "Lazy Initialization": Only allocate memory the first time this is requested
        s_instance = new ResourceManager();
    }
    return s_instance; // Return the global pointer
}

unsigned int ResourceManager::compileShader(const char* source, GLenum type) {
    // Ask the GPU to reserve an empty slot for a new shader (either Vertex or Fragment)
    unsigned int shader = glCreateShader(type);
    
    // Give the GPU the raw string of C++ text containing our GLSL code
    glShaderSource(shader, 1, &source, nullptr);
    
    // Tell the GPU to compile that text into runnable machine instructions
    glCompileShader(shader);
    
    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // Check if compilation was successful
    
    if (!success) { // If it failed (usually due to a syntax error in your .glsl file)
        glGetShaderInfoLog(shader, 512, nullptr, infoLog); // Grab the error message from the GPU
        std::cerr << "Shader compilation failed: " << infoLog << std::endl; // Print it so you can fix it
        return 0; // Return 0 to indicate a broken shader
    }
    
    return shader; // Return the valid hardware ID
}

unsigned int ResourceManager::linkProgram(unsigned int vertexShader, unsigned int fragmentShader) {
    // Ask the GPU to create an empty "Program" (which is just a pipeline connecting multiple shaders)
    unsigned int program = glCreateProgram();
    
    // Attach our compiled shaders to this program
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    
    // Link them together. This ensures the 'out' variables of the Vertex shader match the 'in' variables of the Fragment shader.
    glLinkProgram(program);
    
    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success); // Check if linking was successful
    
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
        return 0;
    }
    
    return program; // Return the final, ready-to-use Program ID
}

std::string ResourceManager::readFile(const std::string& path) {
    std::ifstream file(path); // Open a file stream pointing to the path
    
    if (!file.is_open()) { // Safety check: Did we misspell the file name?
        std::cerr << "Failed to open file: " << path << std::endl;
        return ""; // Return an empty string
    }
    
    std::stringstream buffer;
    // C++ trick: rdbuf() gets the raw memory buffer of the file. 
    // Piping it into a stringstream is the fastest way to read a whole text file in standard C++.
    buffer << file.rdbuf(); 
    
    return buffer.str(); // Convert the stream to a standard std::string and return it
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string& name, const std::string& vertPath, const std::string& fragPath) {
    // 1. Read the GLSL files from the hard drive
    std::string vertexCode = readFile(vertPath);
    std::string fragmentCode = readFile(fragPath);
    
    if (vertexCode.empty() || fragmentCode.empty()) {
        return nullptr; // Fail safely if files are missing
    }
    
    // 2. Compile both pieces of text into GPU shaders
    unsigned int vertexShader = compileShader(vertexCode.c_str(), GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentCode.c_str(), GL_FRAGMENT_SHADER);
    
    if (vertexShader == 0 || fragmentShader == 0) {
        return nullptr; // Fail safely if GLSL syntax is broken
    }
    
    // 3. Link them into a final usable program
    unsigned int program = linkProgram(vertexShader, fragmentShader);
    
    // 4. Cleanup: We don't need the standalone shaders anymore now that they are baked into the Program
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (program == 0) return nullptr;
    
    // 5. Wrap the raw OpenGL ID inside our C++ Shader object using a Shared Pointer
    auto shader = std::make_shared<Shader>();
    shader->ID = program;
    
    // 6. Cache it in our Hash Map! Now `getShader("basic")` will return this instantly.
    m_shaders[name] = shader;
    
    return shader;
}

std::shared_ptr<Shader> ResourceManager::getShader(const std::string& name) {
    auto it = m_shaders.find(name); // Search the Hash Map for the string key
    if (it != m_shaders.end()) {
        return it->second; // Found it! Return the mapped value (the shader pointer)
    }
    return nullptr; // Not found
}

std::shared_ptr<Texture2D> ResourceManager::loadTexture(const std::string& name, const std::string& path) {
    // 1. CACHE CHECK: Did we already load this image previously?
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        return it->second; // Yes! Return it immediately to save CPU time and RAM.
    }

    // Since our 2D camera puts Y=0 at the Top (like standard 2D games), we don't flip the image.
    stbi_set_flip_vertically_on_load(false);

    int width, height, channels;
    
    // 2. STB IMAGE MAGIC: Decodes the PNG/JPG file bytes into a massive array of color data
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return nullptr;
    }

    // Determine if the image has an Alpha (transparency) channel (4 channels) or just RGB (3 channels)
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    // 3. UPLOAD TO GPU
    unsigned int id;
    glGenTextures(1, &id); // Ask GPU for a blank texture slot
    glBindTexture(GL_TEXTURE_2D, id); // Make it the active texture

    // Set wrapping to 'clamp' so textures don't tile or bleed out of bounds
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Set filtering to 'linear' (smooth). Change to GL_NEAREST if you want blocky pixel-art!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Send the raw byte array from CPU RAM to the GPU VRAM
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    
    // Tell OpenGL to generate scaled down versions (mipmaps) automatically
    glGenerateMipmap(GL_TEXTURE_2D); 

    // 4. CLEANUP
    stbi_image_free(data); // Destroy the CPU copy of the image bytes, we only need the GPU copy now!
    glBindTexture(GL_TEXTURE_2D, 0); // Unbind

    // 5. Wrap the ID in our C++ object
    auto tex = std::make_shared<Texture2D>();
    tex->ID = id;
    tex->width = width;
    tex->height = height;

    // 6. Save to cache
    m_textures[name] = tex;
    
    return tex;
}

std::shared_ptr<Texture2D> ResourceManager::getTexture(const std::string& name) {
    auto it = m_textures.find(name); // Search the Hash Map
    if (it != m_textures.end()) return it->second; // Return it if found
    return nullptr;
}
#pragma once // Tells the compiler to only include this file once per compilation unit.

#include <vector> // Includes the C++ standard library dynamic array (vector).
#include <glm/glm.hpp> // Includes GLM for math types like vectors (vec2, vec4).
#include "Renderer.h" // Includes your custom Renderer class so we can draw the particles.

// Struct representing a single, individual particle in the world.
struct Particle { 
    glm::vec2 Position; // The X and Y coordinates of the particle.
    glm::vec2 Velocity; // The speed and direction the particle is moving per second.
    glm::vec4 Color;    // The RGBA (Red, Green, Blue, Alpha/Transparency) color of the particle.
    float Life;         // How much time (in seconds) the particle has left before it "dies".
    float Size;         // The width and height of the particle.
    
    // Constructor: Sets safe default values when a Particle is first created in memory.
    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f), Size(1.0f) { } 
};

// Class that manages a large group (pool) of particles and handles their physics and rendering.
class ParticleSystem { 
private: // Private members cannot be accessed outside this class.
    std::vector<Particle> m_particles; // The dynamic array storing all our particle objects.
    unsigned int m_amount;             // The maximum number of particles allowed to exist at once.

public: // Public members can be called by your Game/State classes.
    // Constructor taking the maximum amount of particles you want to pre-allocate.
    ParticleSystem(unsigned int amount); 

    // Called every frame to move particles and decrease their lifespan.
    void Update(float dt); 

    // Called every frame to send the living particles to the Renderer.
    void Draw(Renderer* renderer); 

    // Wakes up 'count' dead particles, places them at 'position', and gives them movement/color.
    void Emit(unsigned int count, const glm::vec2& position, const glm::vec2& baseVelocity = glm::vec2(0.0f), const glm::vec4& color = glm::vec4(1.0f)); 
    
private: 
    // A helper function to search through our vector and find a particle whose Life is <= 0.
    unsigned int firstUnusedParticle(); 
};
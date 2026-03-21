#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Renderer.h"

// Struct for an individual particle
struct Particle {
    glm::vec2 Position;
    glm::vec2 Velocity;
    glm::vec4 Color;
    float Life;
    float Size;
    
    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f), Size(1.0f) { }
};

// Simple particle system to handle managing and drawing particles
class ParticleSystem {
private:
    std::vector<Particle> m_particles;
    unsigned int m_amount;

public:
    ParticleSystem(unsigned int amount);

    // Call this each frame to update the logic of all particles
    void Update(float dt);

    // Call this each frame to render all currently living particles
    void Draw(Renderer* renderer);

    // Spawn new particles at a given position
    void Emit(unsigned int count, const glm::vec2& position, const glm::vec2& baseVelocity = glm::vec2(0.0f), const glm::vec4& color = glm::vec4(1.0f));
    
private:
    // Finds a dead particle in the array
    unsigned int firstUnusedParticle();
};

#include "ParticleSystem.h"
#include <cstdlib>

ParticleSystem::ParticleSystem(unsigned int amount)
    : m_amount(amount)
{
    // Pre-allocate the particles
    for (unsigned int i = 0; i < m_amount; ++i) {
        m_particles.push_back(Particle());
    }
}

void ParticleSystem::Update(float dt)
{
    for (unsigned int i = 0; i < m_amount; ++i) {
        Particle& p = m_particles[i];
        p.Life -= dt; 
        if (p.Life > 0.0f) {
            // Apply velocity
            p.Position += p.Velocity * dt;
            // Slowly fade out the color's alpha based on life
            p.Color.a -= dt * 2.5f;
            // Slowly shrink the particle
            p.Size -= dt * 10.0f;
            if (p.Size < 0.0f) p.Size = 0.0f;
        }
    }
}

void ParticleSystem::Draw(Renderer* renderer)
{
    // Draw all particles that are currently alive
    for (unsigned int i = 0; i < m_amount; ++i) {
        Particle& p = m_particles[i];
        if (p.Life > 0.0f) {
            renderer->drawRect(p.Position, glm::vec2(p.Size, p.Size), p.Color);
        }
    }
}

void ParticleSystem::Emit(unsigned int count, const glm::vec2& position, const glm::vec2& baseVelocity, const glm::vec4& color)
{
    for (unsigned int i = 0; i < count; ++i) {
        unsigned int unused = firstUnusedParticle();
        
        // Randomize the start parameters slightly for visual variation
        float randomX = ((rand() % 100) - 50) / 10.0f;
        float randomY = ((rand() % 100) - 50) / 10.0f;
        float randomColorVariation = ((rand() % 100) / 100.0f) * 0.2f; // Slight color jitter
        
        Particle& p = m_particles[unused];
        p.Position = position;
        p.Color = glm::vec4(color.r + randomColorVariation, 
                            color.g + randomColorVariation, 
                            color.b + randomColorVariation, 
                            color.a); // Use alpha from passed color
        p.Life = 0.8f + ((rand() % 40) / 100.0f); // Live for 0.8-1.2 seconds
        p.Velocity = baseVelocity + glm::vec2(randomX, randomY) * 25.0f; // Add randomness to velocity, stronger spread
        p.Size = 8.0f + (rand() % 8); // Start size around 8-15
    }
}

unsigned int ParticleSystem::firstUnusedParticle()
{
    // First, search from end (often a small optimization)
    // Actually, for simplicity, let's just linearly search
    for (unsigned int i = 0; i < m_amount; ++i) {
        if (m_particles[i].Life <= 0.0f) {
            return i;
        }
    }
    // If none are dead, just override the first one
    return 0;
}

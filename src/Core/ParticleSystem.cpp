#include "ParticleSystem.h" // Include the header file we just defined.
#include <cstdlib>          // Include C standard library for the rand() function.

ParticleSystem::ParticleSystem(unsigned int amount) // Constructor implementation.
    : m_amount(amount) // Initialize our m_amount variable with the passed-in amount.
{
    // C++ trick: Instead of a for-loop with push_back, resize() automatically 
    // creates the exact amount of Particles we need in one go!
    m_particles.resize(m_amount); 
}

void ParticleSystem::Update(float dt) // The update function called every game frame.
{
    for (unsigned int i = 0; i < m_amount; ++i) { // Loop through absolutely every particle in the pool.
        Particle& p = m_particles[i]; // Get a reference (&) to the particle so we can modify the original.
        p.Life -= dt; // Subtract the frame time (dt) from the particle's life.
        
        if (p.Life > 0.0f) { // If the particle is still "alive"...
            // Apply velocity: New Position = Old Position + (Velocity * Time).
            p.Position += p.Velocity * dt; 
            
            // Slowly fade out the color's transparency (alpha) so it disappears smoothly.
            p.Color.a -= dt * 2.5f; 
            
            // Slowly shrink the particle's size so it gets smaller over time.
            p.Size -= dt * 10.0f; 
            
            // Clamp the size so it never becomes negative (which could flip the texture/drawing).
            if (p.Size < 0.0f) p.Size = 0.0f; 
        }
    }
}

void ParticleSystem::Draw(Renderer* renderer) // The draw function.
{
    for (unsigned int i = 0; i < m_amount; ++i) { // Loop through all particles again.
        Particle& p = m_particles[i]; // Get a reference to the current particle.
        
        if (p.Life > 0.0f) { // We ONLY draw particles that are currently alive.
            // Tell the renderer to draw a rectangle at the particle's position, size, and color.
            renderer->drawRect(p.Position, glm::vec2(p.Size, p.Size), p.Color); 
        }
    }
}

void ParticleSystem::Emit(unsigned int count, const glm::vec2& position, const glm::vec2& baseVelocity, const glm::vec4& color)
{
    for (unsigned int i = 0; i < count; ++i) { // Loop 'count' times to spawn the requested number of particles.
        unsigned int unused = firstUnusedParticle(); // Ask our helper function for the index of a "dead" particle.
        
        // MATH EXPLANATION: rand() % 100 gives a number from 0 to 99. 
        // Subtracting 50 gives -50 to 49. Dividing by 10.0f gives a float between -5.0f and 4.9f.
        float randomX = ((rand() % 100) - 50) / 10.0f; // Random horizontal speed offset.
        float randomY = ((rand() % 100) - 50) / 10.0f; // Random vertical speed offset.
        
        // Similar math: creates a tiny random value between 0.0 and 0.198 to slightly jitter the color.
        float randomColorVariation = ((rand() % 100) / 100.0f) * 0.2f; 
        
        Particle& p = m_particles[unused]; // Grab the dead particle we are about to recycle.
        p.Position = position; // Move it to the requested spawn point (e.g., the snake's tail).
        
        // Set the color, adding our slight random variation to Red, Green, and Blue so they don't look identical.
        p.Color = glm::vec4(color.r + randomColorVariation, 
                            color.g + randomColorVariation, 
                            color.b + randomColorVariation, 
                            color.a); // Keep the original alpha (transparency).
                            
        // Give it a random lifespan between 0.8 and 1.19 seconds.
        p.Life = 0.8f + ((rand() % 40) / 100.0f); 
        
        // Set its starting speed: The base speed + our random offsets multiplied by 25 for dramatic spread.
        p.Velocity = baseVelocity + glm::vec2(randomX, randomY) * 25.0f; 
        
        // Set a random starting size between 8.0 and 15.0 pixels.
        p.Size = 8.0f + (rand() % 8); 
    }
}

unsigned int ParticleSystem::firstUnusedParticle() // Helper function to find a dead particle.
{
    // Search the array linearly from start to finish.
    for (unsigned int i = 0; i < m_amount; ++i) { 
        if (m_particles[i].Life <= 0.0f) { // If the particle's life is 0 or less, it's dead/unused.
            return i; // Return this index so Emit() can recycle it.
        }
    }
    // If we looped through the whole array and EVERYTHING is currently alive,
    // we just overwrite the very first particle (index 0) so the game doesn't crash.
    return 0; 
}
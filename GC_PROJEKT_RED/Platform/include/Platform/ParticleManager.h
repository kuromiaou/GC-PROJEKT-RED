#pragma once

#include "Particle.h"

#include <algorithm>
#include <vector>

class ParticleSystem {
public:
    std::vector<Particle> particles;

    void emit(const Particle& particle) {
        particles.push_back(particle);
    }

    void update(float deltaTime) {
        for (auto& particle : particles) {
            particle.update(deltaTime);
        }
        particles.erase(
            std::remove_if(particles.begin(), particles.end(), [](const Particle& p) { return p.lifetime <= 0; }),
            particles.end()
        );
    }
};
#pragma once

struct Particle {
	float position[3];
	float velocity[3];
	float color[4];
	float lifetime;
	float initialLifetime;


	Particle() : position{ 0, 0, 0 }, velocity{ 0, 0, 0 }, color{ 1, 1, 1, 1 }, lifetime(0), initialLifetime(0) {}

	Particle(float x, float y, float z, float vx, float vy, float vz,
		float r, float g, float b, float a, float lifetime)
		: position{ x, y, z }, velocity{ vx, vy, vz }, color{ r, g, b, a },
		lifetime(lifetime), initialLifetime(lifetime) {
	}
	void update(float deltaTime) {
		position[0] += velocity[0] * deltaTime;
		position[1] += velocity[1] * deltaTime;
		position[2] += velocity[2] * deltaTime;
		lifetime -= deltaTime;
	}
};
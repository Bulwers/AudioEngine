#pragma once

#include <SDL3/SDL_stdinc.h>

#include <vector>

class ITickable;

class TickSystem
{
public:

	void initialize();

	void addTickable(ITickable* tickable);
	void removeTickable(ITickable* tickable);
	
	void update();

	float getDeltaTime() const { return deltaTime; }


private:
		
	std::vector<ITickable*> tickables;


	Uint64 lastTickTime = 0;
	double frequency = 1.0;

	float deltaTime = 0.0f;
	void processTick(float deltaTime);
};
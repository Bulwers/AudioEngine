#include <algorithm>
#include <SDL3/SDL_timer.h>

#include "TickSystem.h"
#include "ITickable.h"

void TickSystem::initialize()
{
	lastTickTime = SDL_GetPerformanceCounter();
	frequency = static_cast<double>(SDL_GetPerformanceFrequency());
	deltaTime = 0.0f;
}

void TickSystem::addTickable(ITickable* tickable)
{
	tickables.push_back(tickable);
}

void TickSystem::removeTickable(ITickable* tickable)
{
	tickables.erase(std::remove(tickables.begin(), tickables.end(), tickable), tickables.end());
}

void TickSystem::update()
{
	const Uint64 currentTickTime = SDL_GetPerformanceCounter();
	const Uint64 elapsedTicks = currentTickTime - lastTickTime;
	lastTickTime = currentTickTime;

	deltaTime = static_cast<float>(static_cast<double>(elapsedTicks) /frequency);

	processTick(deltaTime);
}

void TickSystem::processTick(float deltaTime)
{
	for (auto& tickable : tickables)
	{
		tickable->tick(deltaTime);
	}
}
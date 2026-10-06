#pragma once

class TickSystem;

class ITickable
{
public:

	explicit ITickable(TickSystem& tickSystem);
	~ITickable();

	// Called every frame of the application
	virtual void tick(float deltaTime) = 0;

private:

	TickSystem& tickSystem;
};
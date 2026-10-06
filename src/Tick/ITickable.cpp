#include "ITickable.h"
#include "TickSystem.h"

ITickable::ITickable(TickSystem& tickSystem) : tickSystem(tickSystem)
{
	tickSystem.addTickable(this);
}

ITickable::~ITickable()
{
	tickSystem.removeTickable(this);
}
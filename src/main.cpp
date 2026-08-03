#include <iostream>

#include "ResourceManager.h"

int main()
{
	ResourceManager resourceManager;

	resourceManager.loadAudioBuffers();
	resourceManager.getAudioBuffers();

    std::cin.get(); // Wait for Enter

    return 0;
}
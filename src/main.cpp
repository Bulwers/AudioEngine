#include <iostream>
#include <memory>

#include <SDL3/SDL.h>    

#include "ResourceManager.h"
#include "AudioMixer.h"
#include "AudioSource.h"
#include "AudioDevice.h"

int main()
{
    if (!SDL_Init(SDL_INIT_AUDIO))
    {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << '\n';
        return 1;
    }

    try
    {
        ResourceManager resourceManager;

		resourceManager.loadAudioBuffers();
        resourceManager.createAudioClips();

		resourceManager.printAudioBuffers();
		resourceManager.printAudioClips();

        auto clip1 = resourceManager.getAudioClip("Test1");
		auto clip2 = resourceManager.getAudioClip("Test2");

        if (!clip1)
        {
			std::cerr << "Failed to load audio clip\n";
			SDL_Quit();

            return 1;
        }
        if (!clip2)
        {
            std::cerr << "Failed to load audio clip\n";
            SDL_Quit();

            return 1;
        }

		auto source1 = std::make_shared<AudioSource>(clip1);
		auto source2 = std::make_shared<AudioSource>(clip2);

		AudioMixer mixer;
		mixer.addSource(source2);
		mixer.addSource(source1);

		AudioDevice audioDevice;

        if (!audioDevice.initialize(&mixer))
        {
			std::cerr << "Failed to initialize audio device\n";
            SDL_Quit();

            return 1;
        }

		source1->setLooping(true);
		source2->setLooping(true);
        source1->play();
        source2->play();

        std::cout << "Press ENTER to quit.\n";
        std::cin.get();

        audioDevice.shutdown();
    }

    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        SDL_Quit();

        return 1;
    }

	catch (...)
    {
        std::cerr << "Unknown error occurred\n";
        SDL_Quit();

        return 1;
    }

    SDL_Quit();

    return 0;
}
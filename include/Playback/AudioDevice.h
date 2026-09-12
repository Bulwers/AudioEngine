#pragma once

#include <SDL3/SDL.h>

#include "AudioMixer.h"
#include <vector>

class AudioDevice
{
public:

	AudioDevice() = default;
	~AudioDevice();

	bool initialize(AudioMixer* mixer);
	void shutdown();

	bool isInitialized() const;

	bool refreshDevice();

	void setBufferSize(int newBufferSize);
	int getBufferSize() const;

private:

	static void audioCallback(
		void* userdata,
		SDL_AudioStream* stream,
		int additionalAmount,
		int totalAmount);

	AudioMixer* audioMixer = nullptr;
	SDL_AudioStream* stream = nullptr;

	std::vector<float> mixBuffer;

	int sampleRate = 44100;
	int	channels = 2;
	int bufferSize = 512;
};
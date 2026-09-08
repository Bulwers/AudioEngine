#pragma once

#include "AudioSource.h"

class AudioMixer
{
public:

	void addSource(std::shared_ptr<AudioSource> source);
	void removeSource(std::shared_ptr<AudioSource> source);

	void mixAudio(float* outputBuffer, size_t numFrames, size_t numChannels);

private:

	std::vector<std::shared_ptr<AudioSource>> sources;
};
#pragma once

#include "AudioMixer.h"
#include "ResourceManager.h"

class AudioEngine
{
public:

	AudioEngine() = default;

	void loadAudioAssets();

	std::shared_ptr<AudioSource> createAudioSource(const std::string& clipname);
	std::vector<std::shared_ptr<AudioSource>>& getAudioSources() { return sources; }
	void removeAudioSource(std::shared_ptr<AudioSource> source);

	ResourceManager& getResourceManager() { return resourceManager; }
	AudioMixer& getAudioMixer() { return audioMixer; }

private:

	ResourceManager resourceManager;
	AudioMixer audioMixer;

	std::vector<std::shared_ptr<AudioSource>> sources;
};

#pragma once

#include "AudioMixer.h"
#include "ResourceManager.h"
#include "AudioDevice.h"

class AudioEngine
{
public:

	AudioEngine() = default;
	~AudioEngine();

	bool initialize();
	void shutdown();

	void loadAudioAssets();

	bool isInitialized() const { return initialized; }

	std::shared_ptr<AudioSource> createAudioSource(const std::string& clipname);

	ResourceManager& getResourceManager() { return resourceManager; }
	AudioMixer& getAudioMixer() { return audioMixer; }
	AudioDevice& getAudioDevice() { return audioDevice; }

private:

	ResourceManager resourceManager;
	AudioMixer audioMixer;
	AudioDevice audioDevice;

	std::vector<std::shared_ptr<AudioSource>> sources;

	bool initialized = false;
};
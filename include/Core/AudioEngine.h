#pragma once

#include "AudioMixer.h"
#include "ResourceManager.h"
#include "AudioDevice.h"

class TickSystem;

class AudioEngine
{
public:

	AudioEngine(TickSystem* tickSystem);
	~AudioEngine();

	bool initialize();
	void shutdown();

	void loadAudioAssets();

	bool isInitialized() const { return initialized; }

	std::shared_ptr<AudioSource> createAudioSource(const std::string& clipname);

	ResourceManager& getResourceManager() { return resourceManager; }
	AudioMixer& getAudioMixer() { return audioMixer; }
	AudioDevice& getAudioDevice() { return audioDevice; }
	TickSystem& getTickSystem() { return *tickSystem; }

private:

	ResourceManager resourceManager;
	AudioMixer audioMixer;
	AudioDevice audioDevice;

	TickSystem* tickSystem = nullptr;

	std::vector<std::shared_ptr<AudioSource>> sources;

	bool initialized = false;
};
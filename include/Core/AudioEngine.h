#pragma once

#include "AudioMixer.h"
#include "ResourceManager.h"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

class AudioEngine
{
public:

	explicit AudioEngine(AudioFormat format = {}, AudioResourceConfig resourceConfig = {});

	void loadAudioAssets();

	std::shared_ptr<AudioSource> createAudioSource(const std::string& clipname);
	std::vector<std::shared_ptr<AudioSource>> getAudioSources();
	bool removeAudioSource(const std::shared_ptr<AudioSource>& source);

	ResourceManager& getResourceManager() { return resourceManager; }
	AudioMixer& getAudioMixer() { return audioMixer; }

private:

	void collectRetiredSources();

	ResourceManager resourceManager;
	AudioMixer audioMixer;

	std::mutex sourcesMutex;
	std::vector<std::shared_ptr<AudioSource>> sources;
};

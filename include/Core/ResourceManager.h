#pragma once

#include "AudioClip.h"
#include "AudioBuffer.h"

#include "WavLoader.h"

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>


class ResourceManager
{
public:

	ResourceManager();

	void createAudioDirectories();

	void createAudioClips();

	void saveAudioClips();
	void loadAudioBuffers();
	void loadAudioClips();

	std::shared_ptr<AudioBuffer> getAudioBuffer(const std::string& filePath);
	std::shared_ptr<AudioClip> getAudioClip(const std::string& name);

	const std::unordered_map<std::string, std::shared_ptr<AudioClip>>& getAudioClips() const { return audioClips; }

	void printAudioBuffers() const;
	void printAudioClips() const;

private:

	std::vector<std::string> requiredDirs = {
			"Assets",
			"Assets/SoundSrc",
			"Assets/AudioClips",
			"Assets/AudioClips/Ambient",
			"Assets/AudioClips/Music",
			"Assets/AudioClips/Other",
			"Assets/AudioClips/SFX",
			"Assets/AudioClips/UI",
			"Assets/AudioClips/Voice"
	};
	
	WavLoader wavLoader;

	std::unordered_map<std::string, std::shared_ptr<AudioClip>> audioClips;
	std::unordered_map<std::string, std::shared_ptr<AudioBuffer>> audioBuffers;
};
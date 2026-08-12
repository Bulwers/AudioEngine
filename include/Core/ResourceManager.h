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

	void checkAudioDirectories();

	void createAudioClips();

	void saveAudioClips();
	void loadAudioBuffers();
	void loadAudioClips();

	std::shared_ptr<AudioBuffer> getAudioBuffer(const std::string& filePath);
	std::shared_ptr<AudioClip> getAudioClip(const std::string& name);

	void getAudioBuffers();
	void getAudioClips();

private:

	WavLoader wavLoader;

	std::unordered_map<std::string, std::shared_ptr<AudioClip>> audioClips;
	std::unordered_map<std::string, std::shared_ptr<AudioBuffer>> audioBuffers;
};
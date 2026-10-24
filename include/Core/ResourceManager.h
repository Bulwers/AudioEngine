#pragma once
#pragma once

#include "AudioClip.h"
#include "AudioBuffer.h"

#include "WavLoader.h"
#include "AudioResourceConfig.h"

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>


class ResourceManager
{
public:

	explicit ResourceManager(AudioResourceConfig config = {});

	void createAudioDirectories();

	void createAudioClips();

	void saveAudioClips();
	void loadAudioBuffers();
	void loadAudioClips();
	std::vector<std::string> getAvailableWavFiles() const;
	bool createAndSaveAudioClip(const std::string& wavFilePath, const std::string& clipName, AudioCategory category, std::string& errorMessage);
	bool removeAudioClipFile(const std::string& clipName, std::string& errorMessage);

	std::shared_ptr<AudioBuffer> getAudioBuffer(const std::string& filePath);
	std::shared_ptr<AudioClip> getAudioClip(const std::string& name);

	const std::unordered_map<std::string, std::shared_ptr<AudioClip>>& getAudioClips() const { return audioClips; }
	const AudioResourceConfig& getConfig() const { return config; }

private:

	std::filesystem::path getSourcePath() const;
	std::filesystem::path getClipPath() const;
	std::filesystem::path getClipFilePath(AudioCategory category, const std::string& clipName) const;

	AudioResourceConfig config;
	
	WavLoader wavLoader;

	std::unordered_map<std::string, std::shared_ptr<AudioClip>> audioClips;
	std::unordered_map<std::string, std::shared_ptr<AudioBuffer>> audioBuffers;
};

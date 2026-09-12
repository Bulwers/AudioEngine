#pragma once

#include "AudioBuffer.h"

#include <string>
#include <memory>

enum class AudioCategory
{
	Music = 1,
	SFX = 2,
	UI = 3,
	Ambient = 4,
	Voice = 5,
	Other = 0
};

class AudioClip
{
public:

	AudioClip() = default;
	AudioClip(std::string name, std::string filePath, std::shared_ptr<AudioBuffer> buffer);
	AudioClip(
		std::string name, 
		std::string filePath, 
		AudioCategory category,
		float volume,
		bool looping, 
		size_t loopStartFrame,
		size_t loopEndFrame);

	const std::string& getName() const { return name; }
	const std::string& getFilePath() const { return filePath; }
	const std::shared_ptr<AudioBuffer>& getBuffer() const { return buffer; }

	double getDuration() const { return buffer ? buffer->getDuration() : 0.0; }
	AudioCategory getCategory() const { return category; }
	float getVolume() const { return volume; }

	void setBuffer(std::shared_ptr<AudioBuffer> newBuffer) { buffer = std::move(newBuffer); }
	void setCategory(AudioCategory newCategory) { category = newCategory; }
	void setName(const std::string& newName) { name = newName; }
	void setVolume(float newVolume) { volume = newVolume; }
	void setLooping(bool loop, size_t startFrame = 0, size_t endFrame = 0);
	bool isLooping() const { return looping; }
	
	void saveToFile(const std::string& clipFilePath) const;
	static AudioClip loadFromFile(const std::string& clipFilePath);

	void printInfo() const;

private:

	std::string name;
	std::string filePath;
	std::shared_ptr<AudioBuffer> buffer;
	AudioCategory category = AudioCategory::Other;
	float volume = 1.0f;
	bool looping = false;
	size_t loopStartFrame = 0;
	size_t loopEndFrame = 0;

};
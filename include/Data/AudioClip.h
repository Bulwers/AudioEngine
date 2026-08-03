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
	const double getDuration() const { return buffer->getDuration(); }
	AudioCategory getCategory() const { return category; }

	void setCategory(AudioCategory newCategory) { category = newCategory; }
	void setName(const std::string& newName) { name = newName; }
	void setVolume(float newVolume) { volume = newVolume; }
	void setLooping(size_t startFrame, size_t endFrame);
	void setLooping(bool loop) { looping = loop; }

	bool isLooping() const { return looping; }
	
	void saveToFile(const std::string& clipFilePath) const;
	AudioClip loadFromFile(const std::string& clipFilePath);

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
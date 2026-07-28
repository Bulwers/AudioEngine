#pragma once

#include "AudioBuffer.h"

#include <string>
#include <memory>

enum class AudioCategory
{
	SFX,
	Music,
	Voice,
	Ambient,
	Other
};

class AudioClip
{
public:

	AudioClip() = default;
	AudioClip(std::string name, std::string filePath, std::shared_ptr<AudioBuffer> buffer);

	const std::string& getName() const { return name; }
	const std::string& getFilePath() const { return filePath; }

	const std::shared_ptr<AudioBuffer>& getBuffer() const { return buffer; }
	const double getDuration() const { return buffer->getDuration(); }
	
	void setLooping(size_t startFrame, size_t endFrame);
	void setLooping(bool loop) { looping = loop; }
	bool isLooping() const { return looping; }
	
private:

	std::shared_ptr<AudioBuffer> buffer;
	std::string name;
	std::string filePath;
	float volume = 1.0f;
	bool looping = false;
	size_t loopStartFrame = 0;
	size_t loopEndFrame = 0;
	AudioCategory category = AudioCategory::Other;

};
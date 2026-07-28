#include "AudioClip.h"

AudioClip::AudioClip(std::string name, std::string filePath, std::shared_ptr<AudioBuffer> buffer)
{
	if (!buffer)
	{
		throw std::invalid_argument("AudioClip buffer cannot be null.");
	}
	this->name = std::move(name);
	this->filePath = std::move(filePath);
	this->buffer = std::move(buffer);
}

void AudioClip::setLooping(size_t startFrame, size_t endFrame)
{
	looping = true;
	loopStartFrame = startFrame;
	loopEndFrame = endFrame;
}

#include "AudioBuffer.h"

#include <stdexcept>

AudioBuffer::AudioBuffer(
	std::string filePath, 
	uint16_t channels,
	uint32_t sampleRate,
	uint16_t bitsPerSample, 
	const std::vector<float>& samples)
		: filePath(std::move(filePath)),
		channels(channels), 
		sampleRate(sampleRate),
		bitsPerSample(bitsPerSample), 
		samples(samples)
{
	if (channels == 0)
	{
		throw std::invalid_argument("AudioBuffer channels must be greater than 0.");
	}
	if (sampleRate == 0)
	{
		throw std::invalid_argument("AudioBuffer sampleRate must be greater than 0.");
	}
	if (samples.size() % channels != 0)
	{
		throw std::invalid_argument("AudioBuffer sample count must be divisible by channel count.");
	}
}

float AudioBuffer::getSample(std::size_t frame, std::size_t channel) const
{
	if (channel >= channels)
	{
		throw std::out_of_range("AudioBuffer channel index is out of range. Getter");
	}
	if (frame >= getFrameCount())
	{
		throw std::out_of_range("AudioBuffer frame index is out of range. Getter");
	}
    return samples[frame * channels + channel];
}

void AudioBuffer::setSample(std::size_t frame, std::size_t channel, float value)
{
	if (channel >= channels)
	{
		throw std::out_of_range("AudioBuffer channel index is out of range. Setter");
	}
	if (frame >= getFrameCount())
	{
		throw std::out_of_range("AudioBuffer frame index is out of range. Setter");
	}

    samples[frame * channels + channel] = value;
}
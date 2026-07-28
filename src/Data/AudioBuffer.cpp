#include "AudioBuffer.h"

#include <cassert>
#include <stdexcept>

AudioBuffer::AudioBuffer(uint16_t channels, uint32_t sampleRate, uint16_t bitsPerSample, const std::vector<float>& audioData)
{
	if (channels == 0)
	{
		throw std::invalid_argument("AudioBuffer channels must be greater than 0.");
	}
	if (sampleRate == 0)
	{
		throw std::invalid_argument("AudioBuffer sampleRate must be greater than 0.");
	}
	if (audioData.size() % channels != 0)
	{
		throw std::invalid_argument("AudioBuffer sample count must be divisible by channel count.");
	}

	this->channels = channels;
	this->sampleRate = sampleRate;
	samples = audioData;

    assert(channels > 0);
    assert(sampleRate > 0);
}

float AudioBuffer::getSample(std::size_t frame, std::size_t channel) const
{
	if (channel >= channels)
	{
		throw std::out_of_range("AudioBuffer channel index is out of range.");
	}
	if (frame >= getFrameCount())
	{
		throw std::out_of_range("AudioBuffer frame index is out of range.");
	}
    assert(channel < channels);
    assert(frame < getFrameCount());

    return samples[frame * channels + channel];
}

void AudioBuffer::setSample(std::size_t frame, std::size_t channel, float value)
{
	if (channel >= channels)
	{
		throw std::out_of_range("AudioBuffer channel index is out of range.");
	}
	if (frame >= getFrameCount())
	{
		throw std::out_of_range("AudioBuffer frame index is out of range.");
	}
    assert(channel < channels);
    assert(frame < getFrameCount());

    samples[frame * channels + channel] = value;
}
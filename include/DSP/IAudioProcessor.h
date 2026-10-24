#pragma once

#include <cstddef>
#include <cstdint>

// Runs on the audio callback thread. Implementations must not allocate, block, or throw.
class IAudioProcessor
{
public:
	virtual ~IAudioProcessor() = default;
	virtual void process(float* interleavedSamples, std::size_t frameCount,
		std::uint16_t channelCount, std::uint32_t sampleRate) noexcept = 0;
};

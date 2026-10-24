#pragma once

#include "IAudioProcessor.h"

#include <array>
#include <memory>

// Immutable after it is submitted to AudioMixer. Effects run in insertion order.
class AudioProcessorChain final : public IAudioProcessor
{
public:
	static constexpr std::size_t MaxProcessors = 16;

	// Configure the chain on a control thread before submitting it to the mixer.
	bool addProcessor(std::shared_ptr<IAudioProcessor> processor) noexcept;
	bool isSealed() const noexcept { return sealed; }

	void process(float* interleavedSamples, std::size_t frameCount,
		std::uint16_t channelCount, std::uint32_t sampleRate) noexcept override;

private:
	friend class AudioMixer;
	void seal() noexcept { sealed = true; }

	std::array<std::shared_ptr<IAudioProcessor>, MaxProcessors> processors{};
	std::size_t processorCount = 0;
	bool sealed = false;
};

#include "AudioProcessorChain.h"

#include <utility>

bool AudioProcessorChain::addProcessor(std::shared_ptr<IAudioProcessor> processor) noexcept
{
	if (sealed || !processor || processorCount >= MaxProcessors)
		return false;

	processors[processorCount++] = std::move(processor);
	return true;
}

void AudioProcessorChain::process(float* interleavedSamples, std::size_t frameCount,
	std::uint16_t channelCount, std::uint32_t sampleRate) noexcept
{
	for (std::size_t i = 0; i < processorCount; ++i)
		processors[i]->process(interleavedSamples, frameCount, channelCount, sampleRate);
}

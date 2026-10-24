#include "AudioMixer.h"

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <utility>

AudioMixer::AudioMixer(AudioFormat format)
	: format(format)
{
	if (format.sampleRate == 0 || format.channels == 0)
	{
		throw std::invalid_argument("AudioMixer format must have a non-zero sample rate and channel count.");
	}
	retainedSources.reserve(MaxSources);
	retainedProcessorChains.reserve(MaxPendingProcessorChainChanges + 1);
}

bool AudioMixer::enqueueAddSource(const std::shared_ptr<AudioSource>& source)
{
	if (!source)
		return false;

	bool expected = false;
	if (!source->addedToMixer.compare_exchange_strong(expected, true, std::memory_order_acq_rel))
		return false;
	source->registeredWithMixer.store(true, std::memory_order_release);

	std::size_t currentCount = scheduledSourceCount.load(std::memory_order_relaxed);
	while (true)
	{
		if (currentCount >= MaxSources)
		{
			source->registeredWithMixer.store(false, std::memory_order_release);
			source->addedToMixer.store(false, std::memory_order_release);
			return false;
		}
		if (scheduledSourceCount.compare_exchange_weak(currentCount, currentCount + 1, std::memory_order_acq_rel))
			break;
	}

	std::lock_guard<std::mutex> lock(commandMutex);
	if (pendingCount == MaxPendingCommands)
	{
		scheduledSourceCount.fetch_sub(1, std::memory_order_acq_rel);
		source->registeredWithMixer.store(false, std::memory_order_release);
		source->addedToMixer.store(false, std::memory_order_release);
		return false;
	}

	retainedSources.push_back(source);
	pendingCommands[pendingWriteIndex] = { SourceCommandType::Add, source.get() };
	pendingWriteIndex = (pendingWriteIndex + 1) % MaxPendingCommands;
	++pendingCount;
	return true;
}

bool AudioMixer::enqueueRemoveSource(const std::shared_ptr<AudioSource>& source)
{
	if (!source)
		return false;

	bool expected = true;
	if (!source->registeredWithMixer.compare_exchange_strong(expected, false, std::memory_order_acq_rel))
		return false;

	std::lock_guard<std::mutex> lock(commandMutex);
	if (pendingCount == MaxPendingCommands)
	{
		source->registeredWithMixer.store(true, std::memory_order_release);
		return false;
	}

	pendingCommands[pendingWriteIndex] = { SourceCommandType::Remove, source.get() };
	pendingWriteIndex = (pendingWriteIndex + 1) % MaxPendingCommands;
	++pendingCount;
	scheduledSourceCount.fetch_sub(1, std::memory_order_acq_rel);
	return true;
}

bool AudioMixer::popRetiredSource(std::shared_ptr<AudioSource>& source)
{
	std::lock_guard<std::mutex> lock(commandMutex);
	if (retiredCount == 0)
		return false;

	AudioSource* retired = retiredSources[retiredReadIndex];
	retiredReadIndex = (retiredReadIndex + 1) % MaxSources;
	--retiredCount;
	const auto it = std::find_if(retainedSources.begin(), retainedSources.end(),
		[retired](const std::shared_ptr<AudioSource>& candidate)
		{
			return candidate.get() == retired;
		});
	if (it == retainedSources.end())
	{
		source.reset();
		return false;
	}
	source = std::move(*it);
	retainedSources.erase(it);
	return true;
}

bool AudioMixer::setMasterProcessorChain(std::shared_ptr<AudioProcessorChain> chain)
{
	collectRetiredProcessorChains();
	std::lock_guard<std::mutex> lock(commandMutex);
	if (pendingProcessorCount == MaxPendingProcessorChainChanges)
		return false;

	if (chain)
	{
		retainedProcessorChains.push_back(chain);
		chain->seal();
	}
	pendingProcessorChains[pendingProcessorWriteIndex] = chain.get();
	pendingProcessorWriteIndex = (pendingProcessorWriteIndex + 1) % MaxPendingProcessorChainChanges;
	++pendingProcessorCount;
	return true;
}

bool AudioMixer::setAudioProcessor(std::shared_ptr<IAudioProcessor> processor)
{
	if (!processor)
		return setMasterProcessorChain(nullptr);

	auto chain = std::make_shared<AudioProcessorChain>();
	if (!chain->addProcessor(std::move(processor)))
		return false;
	return setMasterProcessorChain(std::move(chain));
}

std::size_t AudioMixer::collectRetiredProcessorChains()
{
	std::vector<std::shared_ptr<AudioProcessorChain>> releaseOnControlThread;
	{
		std::lock_guard<std::mutex> lock(commandMutex);
		releaseOnControlThread.reserve(retiredProcessorCount);
		while (retiredProcessorCount > 0)
		{
			AudioProcessorChain* retired = retiredProcessorChains[retiredProcessorReadIndex];
			retiredProcessorReadIndex = (retiredProcessorReadIndex + 1) % MaxPendingProcessorChainChanges;
			--retiredProcessorCount;

			const auto it = std::find_if(retainedProcessorChains.begin(), retainedProcessorChains.end(),
				[retired](const std::shared_ptr<AudioProcessorChain>& candidate)
				{
					return candidate.get() == retired;
				});
			if (it != retainedProcessorChains.end())
			{
				releaseOnControlThread.push_back(std::move(*it));
				retainedProcessorChains.erase(it);
			}
		}
	}
	return releaseOnControlThread.size();
}

std::size_t AudioMixer::copyVisualizationSamples(float* destination, std::size_t maxSamples) const noexcept
{
	if (!destination || maxSamples == 0)
		return 0;

	const std::uint64_t publishedCount = visualizationWriteCount.load(std::memory_order_acquire);
	const std::size_t count = static_cast<std::size_t>(std::min<std::uint64_t>(
		std::min<std::uint64_t>(publishedCount, VisualizationCapacity), maxSamples));
	const std::uint64_t firstSequence = publishedCount - count;
	for (std::size_t i = 0; i < count; ++i)
	{
		const std::uint64_t sampleSequence = firstSequence + i;
		const std::size_t index = static_cast<std::size_t>(sampleSequence % VisualizationCapacity);
		const std::uint64_t expectedMarker = sampleSequence + 1;
		const std::uint64_t before = visualizationSequences[index].load(std::memory_order_acquire);
		if (before != expectedMarker)
			return 0;
		destination[i] = visualizationSamples[index].load(std::memory_order_relaxed);
		const std::uint64_t after = visualizationSequences[index].load(std::memory_order_acquire);
		if (after != expectedMarker)
			return 0;
	}
	return count;
}

void AudioMixer::processPendingCommands() noexcept
{
	std::unique_lock<std::mutex> lock(commandMutex, std::try_to_lock);
	if (!lock.owns_lock())
		return;

	std::size_t processedCommands = 0;
	while (pendingCount > 0 && processedCommands < MaxCommandsPerBlock)
	{
		const SourceCommand command = pendingCommands[pendingReadIndex];
		if (command.type == SourceCommandType::Remove && retiredCount == MaxSources)
			break;

		pendingReadIndex = (pendingReadIndex + 1) % MaxPendingCommands;
		--pendingCount;
		++processedCommands;

		if (command.type == SourceCommandType::Add)
		{
			if (activeSourceCount < MaxSources)
				activeSources[activeSourceCount++] = command.source;
		}
		else
		{
			removeActiveSource(command.source);
			retiredSources[retiredWriteIndex] = command.source;
			retiredWriteIndex = (retiredWriteIndex + 1) % MaxSources;
			++retiredCount;
		}
	}

	if (pendingProcessorCount > 0 && retiredProcessorCount < MaxPendingProcessorChainChanges)
	{
		AudioProcessorChain* nextChain = pendingProcessorChains[pendingProcessorReadIndex];
		pendingProcessorReadIndex = (pendingProcessorReadIndex + 1) % MaxPendingProcessorChainChanges;
		--pendingProcessorCount;

		if (activeProcessorChain)
		{
			retiredProcessorChains[retiredProcessorWriteIndex] = activeProcessorChain;
			retiredProcessorWriteIndex = (retiredProcessorWriteIndex + 1) % MaxPendingProcessorChainChanges;
			++retiredProcessorCount;
		}
		activeProcessorChain = nextChain;
	}
}

void AudioMixer::removeActiveSource(AudioSource* source) noexcept
{
	for (std::size_t i = 0; i < activeSourceCount; ++i)
	{
		if (activeSources[i] != source)
			continue;

		activeSources[i] = activeSources[activeSourceCount - 1];
		activeSources[--activeSourceCount] = nullptr;
		return;
	}
}

void AudioMixer::applyPendingCommand(AudioSource& source) noexcept
{
	const auto command = source.pendingCommand.exchange(AudioSource::PendingCommand::None, std::memory_order_acquire);
	const double maxFrame = source.buffer
		? static_cast<double>(source.buffer->getFrameCount())
		: 0.0;
	const double rangeStart = std::clamp(source.playbackRangeStart.load(std::memory_order_relaxed), 0.0, maxFrame);
	const double rangeEnd = std::clamp(source.playbackRangeEnd.load(std::memory_order_relaxed), rangeStart, maxFrame);
	double position = source.playbackPosition.load(std::memory_order_relaxed);

	switch (command)
	{
	case AudioSource::PendingCommand::Play:
		if (!source.clip || !source.buffer || rangeEnd <= rangeStart)
			break;
		if (position < rangeStart || position >= rangeEnd)
			position = rangeStart;
		source.playbackPosition.store(position, std::memory_order_relaxed);
		source.state.store(PlaybackState::Playing, std::memory_order_release);
		break;
	case AudioSource::PendingCommand::Pause:
		if (source.state.load(std::memory_order_relaxed) == PlaybackState::Playing)
			source.state.store(PlaybackState::Paused, std::memory_order_release);
		break;
	case AudioSource::PendingCommand::Stop:
		source.playbackPosition.store(rangeStart, std::memory_order_relaxed);
		source.state.store(PlaybackState::Stopped, std::memory_order_release);
		break;
	case AudioSource::PendingCommand::Restart:
		if (source.clip && source.buffer && rangeEnd > rangeStart)
		{
			source.playbackPosition.store(rangeStart, std::memory_order_relaxed);
			source.state.store(PlaybackState::Playing, std::memory_order_release);
		}
		break;
	case AudioSource::PendingCommand::Seek:
		position = source.requestedSeekPosition.load(std::memory_order_relaxed);
		source.playbackPosition.store(std::clamp(position, rangeStart, rangeEnd), std::memory_order_relaxed);
		break;
	case AudioSource::PendingCommand::None:
		break;
	}
}

void AudioMixer::mixAudio(float* outputBuffer, std::size_t numFrames) noexcept
{
	const std::size_t numChannels = format.channels;
	if (!outputBuffer || numFrames == 0)
		return;

	std::fill(outputBuffer, outputBuffer + numFrames * numChannels, 0.0f);
	processPendingCommands();

	for (std::size_t sourceIndex = 0; sourceIndex < activeSourceCount; ++sourceIndex)
	{
		AudioSource* source = activeSources[sourceIndex];
		if (!source)
			continue;

		applyPendingCommand(*source);
		if (source->state.load(std::memory_order_acquire) != PlaybackState::Playing)
			continue;

		const auto& clip = source->getClip();
		if (!clip)
			continue;

		const auto& buffer = source->buffer;
		if (!buffer || buffer->getChannels() == 0)
			continue;

		const size_t bufferFrames = buffer->getFrameCount();
		if (bufferFrames == 0)
		{
			source->state.store(PlaybackState::Stopped, std::memory_order_release);
			continue;
		}

		const size_t bufferChannels = static_cast<size_t>(buffer->getChannels());
		const float volume = source->volume.load(std::memory_order_relaxed) * source->clipVolume;
		const double pitch = static_cast<double>(source->pitch.load(std::memory_order_relaxed));
		const double rangeStart = std::clamp(source->playbackRangeStart.load(std::memory_order_relaxed), 0.0, static_cast<double>(bufferFrames));
		const double rangeEnd = std::clamp(source->playbackRangeEnd.load(std::memory_order_relaxed), rangeStart, static_cast<double>(bufferFrames));
		const double rangeLength = rangeEnd - rangeStart;
		const uint32_t sourceSampleRate = buffer->getSampleRate();

		if (pitch <= 0.0 || sourceSampleRate == 0)
			continue;
		const double playbackStep = pitch * static_cast<double>(sourceSampleRate) / static_cast<double>(format.sampleRate);

		if (rangeLength <= 0.0)
		{
			source->playbackPosition.store(rangeStart, std::memory_order_relaxed);
			source->state.store(PlaybackState::Stopped, std::memory_order_release);
			continue;
		}

		double playbackPosition = source->playbackPosition.load(std::memory_order_relaxed);
		if (playbackPosition < rangeStart || playbackPosition > rangeEnd)
			playbackPosition = rangeStart;
		const bool shouldLoop = source->loop.load(std::memory_order_relaxed);
		bool stoppedAtEnd = false;

		for (size_t frame = 0; frame < numFrames; ++frame)
		{
			if (playbackPosition < rangeStart || playbackPosition >= rangeEnd)
			{
				if (shouldLoop)
				{
					const double wrapped = std::fmod(std::max(0.0, playbackPosition - rangeStart), rangeLength);
					playbackPosition = rangeStart + wrapped;
				}
				else
				{
					source->state.store(PlaybackState::Stopped, std::memory_order_release);
					source->playbackPosition.store(rangeStart, std::memory_order_relaxed);
					stoppedAtEnd = true;
					break;
				}
			}
			const size_t sourceFrame = static_cast<size_t>(playbackPosition);

			const size_t frame0 = sourceFrame;
			const double fractional = playbackPosition - static_cast<double>(frame0);
			size_t frame1 = frame0 + 1;

			if (static_cast<double>(frame1) >= rangeEnd)
			{
				if (shouldLoop)
				{
					frame1 = static_cast<size_t>(rangeStart);
				}
				else
				{
					frame1 = frame0;
				}
			}

			const float alpha = static_cast<float>(fractional);

			for (size_t channel = 0; channel < numChannels; ++channel)
			{
				float sampleValue = 0.0f;
				size_t sourceChannel = 0;

				if (bufferChannels == 1) sourceChannel = 0;
				else sourceChannel = std::min(channel, bufferChannels - 1);

				const float sample0 = buffer->getSample(frame0, sourceChannel);
				const float sample1 = buffer->getSample(frame1, sourceChannel);
				sampleValue = sample0 + alpha * (sample1 - sample0);
			
				outputBuffer[frame * numChannels + channel] += sampleValue * volume;
			}
			playbackPosition += playbackStep;
		}

		if (!stoppedAtEnd && source->state.load(std::memory_order_relaxed) == PlaybackState::Playing)
		{
			source->playbackPosition.store(playbackPosition, std::memory_order_release);
		}
	}
	const size_t totalSamples = numFrames * numChannels;
	if (activeProcessorChain)
		activeProcessorChain->process(outputBuffer, numFrames, format.channels, format.sampleRate);

	const std::uint64_t firstVisualizationSequence = visualizationWriteCount.load(std::memory_order_relaxed);
	for (size_t i = 0; i < totalSamples; ++i)
	{
		outputBuffer[i] = std::clamp(outputBuffer[i], -1.0f, 1.0f);
		const std::uint64_t sequence = firstVisualizationSequence + i;
		const std::size_t slot = static_cast<std::size_t>(sequence % VisualizationCapacity);
		visualizationSequences[slot].store(0, std::memory_order_release);
		visualizationSamples[slot].store(outputBuffer[i], std::memory_order_relaxed);
		visualizationSequences[slot].store(sequence + 1, std::memory_order_release);
	}
	visualizationWriteCount.store(firstVisualizationSequence + totalSamples, std::memory_order_release);
}

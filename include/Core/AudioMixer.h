#pragma once

#include "AudioFormat.h"
#include "AudioSource.h"
#include "AudioProcessorChain.h"
#include "IAudioProcessor.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

class AudioMixer
{
public:

	static constexpr std::size_t MaxSources = 256;
	static constexpr std::size_t VisualizationCapacity = 8192;
	static constexpr std::size_t MaxPendingProcessorChainChanges = 32;

	explicit AudioMixer(AudioFormat format = {});

	// These bounded operations may be called by control threads. False means the source/queue limit was reached.
	bool enqueueAddSource(const std::shared_ptr<AudioSource>& source);
	bool enqueueRemoveSource(const std::shared_ptr<AudioSource>& source);
	// Call on a control thread to release a removed source after the callback has stopped using it.
	bool popRetiredSource(std::shared_ptr<AudioSource>& source);

	void mixAudio(float* outputBuffer, std::size_t numFrames) noexcept;

	// Changes apply at an audio-block boundary. The chain runs on the complete master mix.
	// Passing nullptr bypasses all master DSP. Returns false if the bounded command queue is full.
	bool setMasterProcessorChain(std::shared_ptr<AudioProcessorChain> chain);
	// Convenience API for a chain containing a single effect; nullptr bypasses master DSP.
	bool setAudioProcessor(std::shared_ptr<IAudioProcessor> processor);
	// Call from a control thread to destroy chains that the callback has finished using.
	std::size_t collectRetiredProcessorChains();
	// Copies the newest samples in chronological order into caller-owned storage.
	// Returns zero if the callback overwrote the requested window during the copy.
	std::size_t copyVisualizationSamples(float* destination, std::size_t maxSamples) const noexcept;
	std::uint64_t getVisualizationSampleCount() const noexcept { return visualizationWriteCount.load(std::memory_order_acquire); }

	const AudioFormat& getFormat() const { return format; }

private:

	enum class SourceCommandType : uint8_t
	{
		Add,
		Remove
	};

	struct SourceCommand
	{
		SourceCommandType type;
		AudioSource* source;
	};

	static constexpr std::size_t MaxPendingCommands = 512;
	static constexpr std::size_t MaxCommandsPerBlock = 64;

	void processPendingCommands() noexcept;
	void applyPendingCommand(AudioSource& source) noexcept;
	void removeActiveSource(AudioSource* source) noexcept;

	const AudioFormat format;
	std::array<AudioSource*, MaxSources> activeSources{};
	std::size_t activeSourceCount = 0;
	std::vector<std::shared_ptr<AudioSource>> retainedSources;

	std::array<SourceCommand, MaxPendingCommands> pendingCommands{};
	std::size_t pendingReadIndex = 0;
	std::size_t pendingWriteIndex = 0;
	std::size_t pendingCount = 0;

	std::array<AudioSource*, MaxSources> retiredSources{};
	std::size_t retiredReadIndex = 0;
	std::size_t retiredWriteIndex = 0;
	std::size_t retiredCount = 0;

	std::atomic<std::size_t> scheduledSourceCount = 0;
	AudioProcessorChain* activeProcessorChain = nullptr; // Accessed only by the audio callback.
	std::array<AudioProcessorChain*, MaxPendingProcessorChainChanges> pendingProcessorChains{};
	std::size_t pendingProcessorReadIndex = 0;
	std::size_t pendingProcessorWriteIndex = 0;
	std::size_t pendingProcessorCount = 0;
	std::array<AudioProcessorChain*, MaxPendingProcessorChainChanges> retiredProcessorChains{};
	std::size_t retiredProcessorReadIndex = 0;
	std::size_t retiredProcessorWriteIndex = 0;
	std::size_t retiredProcessorCount = 0;
	std::vector<std::shared_ptr<AudioProcessorChain>> retainedProcessorChains;
	std::array<std::atomic<float>, VisualizationCapacity> visualizationSamples{};
	std::array<std::atomic<std::uint64_t>, VisualizationCapacity> visualizationSequences{};
	std::atomic<std::uint64_t> visualizationWriteCount = 0;
	std::mutex commandMutex;
};

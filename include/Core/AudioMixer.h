#pragma once

#include "AudioFormat.h"
#include "AudioSource.h"

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

	explicit AudioMixer(AudioFormat format = {});

	// These bounded operations may be called by control threads. False means the source/queue limit was reached.
	bool enqueueAddSource(const std::shared_ptr<AudioSource>& source);
	bool enqueueRemoveSource(const std::shared_ptr<AudioSource>& source);
	// Call on a control thread to release a removed source after the callback has stopped using it.
	bool popRetiredSource(std::shared_ptr<AudioSource>& source);

	void mixAudio(float* outputBuffer, std::size_t numFrames) noexcept;

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
	std::mutex commandMutex;
};

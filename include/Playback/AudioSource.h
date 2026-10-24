#pragma once

#include "AudioClip.h"

#include <atomic>
#include <cstdint>
#include <memory>

enum class PlaybackState
{
	Stopped,
	Playing,
	Paused
};

class AudioSource
{
public:

	AudioSource() = default;
	explicit AudioSource(std::shared_ptr<AudioClip> clip);

	void play();
	void pause();
	void stop();
	void restart();

	void setVolume(float newVolume);
	void setPitch(float newPitch);
	void setLooping(bool shouldLoop);

	const std::shared_ptr<AudioClip>& getClip() const { return clip; }

	PlaybackState getState() const { return state.load(std::memory_order_acquire); }

	bool isPlaying() const { return getState() == PlaybackState::Playing; }
	bool isPaused() const { return getState() == PlaybackState::Paused; }

	float getVolume() const { return volume.load(std::memory_order_relaxed); }
	float getPitch() const { return pitch.load(std::memory_order_relaxed); }
	bool isLooping() const { return loop.load(std::memory_order_relaxed); }

	double getPlaybackPosition() const;
	void setPlaybackPosition(double position);

	double getPlaybackRangeStart() const { return playbackRangeStart.load(std::memory_order_relaxed); }
	double getPlaybackRangeEnd() const { return playbackRangeEnd.load(std::memory_order_relaxed); }
	void setPlaybackRange(double start, double end);

private:

	friend class AudioMixer;

	enum class PendingCommand : uint8_t
	{
		None,
		Play,
		Pause,
		Stop,
		Restart,
		Seek
	};

	const std::shared_ptr<AudioClip> clip;
	const std::shared_ptr<AudioBuffer> buffer;
	const float clipVolume = 1.0f;
	std::atomic<PlaybackState> state = PlaybackState::Stopped;
	// Playback commands are applied at a block boundary; if several arrive first, the latest command wins.
	std::atomic<PendingCommand> pendingCommand = PendingCommand::None;
	std::atomic<bool> registeredWithMixer = false;
	std::atomic<bool> addedToMixer = false;
	std::atomic<float> volume = 1.0f;
	std::atomic<float> pitch = 1.0f;
	std::atomic<bool> loop = false;

	std::atomic<double> playbackPosition = 0.0;
	std::atomic<double> requestedSeekPosition = 0.0;
	std::atomic<double> playbackRangeStart = 0.0;
	std::atomic<double> playbackRangeEnd = 0.0;
};

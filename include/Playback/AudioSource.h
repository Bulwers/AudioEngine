#pragma once

#include "AudioClip.h"

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

	void setClip(std::shared_ptr<AudioClip> newClip);

	void setVolume(float newVolume);
	void setPitch(float newPitch);
	void setLooping(bool shouldLoop);

	const std::shared_ptr<AudioClip>& getClip() const { return clip; }

	PlaybackState getState() const { return state; }

	bool isPlaying() const { return state == PlaybackState::Playing; }
	bool isPaused() const { return state == PlaybackState::Paused; }

	float getVolume() const { return volume; }
	float getPitch() const { return pitch; }
	bool isLooping() const { return loop; }

	double getPlaybackPosition() const;
	void setPlaybackPosition(double position);

private:

	std::shared_ptr<AudioClip> clip;
	PlaybackState state = PlaybackState::Stopped;
	float volume = 1.0f;
	float pitch = 1.0f;
	bool loop = false;

	double playbackPosition = 0.0;
};
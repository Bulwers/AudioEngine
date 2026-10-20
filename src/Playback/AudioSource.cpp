#include "AudioSource.h"

#include <algorithm>

AudioSource::AudioSource(std::shared_ptr<AudioClip> clip) 
	: clip(std::move(clip)) 
{
	if (this->clip) 
	{
		loop = this->clip->isLooping();
		if (this->clip->getBuffer())
		{
			playbackRangeEnd = static_cast<double>(this->clip->getBuffer()->getFrameCount());
		}
	}
}

void AudioSource::play() 
{
	if (!clip || !clip->getBuffer()) 
		return;

	if (playbackPosition >= static_cast<double>(clip->getBuffer()->getFrameCount()))
	{
		playbackPosition = playbackRangeStart;
	}

	if (playbackPosition < playbackRangeStart || playbackPosition >= playbackRangeEnd)
	{
		playbackPosition = playbackRangeStart;
	}
	state = PlaybackState::Playing;
}

void AudioSource::pause() 
{
	if (state == PlaybackState::Playing) 
	{
		state = PlaybackState::Paused;
	}
}

void AudioSource::stop()
{
	state = PlaybackState::Stopped;
	playbackPosition = playbackRangeStart;
}

void AudioSource::restart() 
{
	playbackPosition = playbackRangeStart;
	state = PlaybackState::Playing;
}

void AudioSource::setClip(std::shared_ptr<AudioClip> newClip) 
{
	clip = std::move(newClip);
	state = PlaybackState::Stopped;
	playbackPosition = 0.0;
	loop = clip ? clip->isLooping() : false;
	playbackRangeStart = 0.0;
	playbackRangeEnd = clip && clip->getBuffer() ? static_cast<double>(clip->getBuffer()->getFrameCount()) : 0.0;
}

void AudioSource::setVolume(float newVolume) 
{
	volume = newVolume;
}

void AudioSource::setPitch(float newPitch) 
{
	pitch = newPitch;
}

void AudioSource::setLooping(bool shouldLoop) 
{
	loop = shouldLoop;
}

double AudioSource::getPlaybackPosition() const
{
	if (!clip || !clip->getBuffer()) 
		return 0;

	return playbackPosition;
}

void AudioSource::setPlaybackPosition(double position) 
{
	if (!clip || !clip->getBuffer()) 
		return;

	const double maxFrame = static_cast<double>(clip->getBuffer()->getFrameCount());
	const double clampedStart = std::clamp(playbackRangeStart, 0.0, maxFrame);
	const double clampedEnd = std::clamp(playbackRangeEnd, clampedStart, maxFrame);

	position = std::clamp(position, clampedStart, clampedEnd);

	playbackPosition = position;
}

void AudioSource::setPlaybackRange(double start, double end)
{
	if (!clip || !clip->getBuffer())
	{
		playbackRangeStart = 0.0;
		playbackRangeEnd = 0.0;
		playbackPosition = 0.0;
		return;
	}

	const double maxFrame = static_cast<double>(clip->getBuffer()->getFrameCount());

	start = std::clamp(start, 0.0, maxFrame);
	end = std::clamp(end, start, maxFrame);

	playbackRangeStart = start;
	playbackRangeEnd = end;

	if (playbackPosition < playbackRangeStart || playbackPosition > playbackRangeEnd)
	{
		playbackPosition = playbackRangeStart;
	}
}
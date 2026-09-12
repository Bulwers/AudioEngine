#include "AudioSource.h"

AudioSource::AudioSource(std::shared_ptr<AudioClip> clip) 
	: clip(std::move(clip)) 
{
	if (this->clip) 
	{
		loop = this->clip->isLooping();
	}
}

void AudioSource::play() 
{
	if (!clip || !clip->getBuffer()) 
		return;

	if (playbackPosition >= static_cast<double>(clip->getBuffer()->getFrameCount()))
	{
		playbackPosition = 0.0;
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
	playbackPosition = 0.0;
}

void AudioSource::restart() 
{
	playbackPosition = 0.0;
	state = PlaybackState::Playing;
}

void AudioSource::setClip(std::shared_ptr<AudioClip> newClip) 
{
	clip = std::move(newClip);
	state = PlaybackState::Stopped;
	playbackPosition = 0.0;
	loop = clip ? clip->isLooping() : false;
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

	if (position < 0.0) 
		position = 0.0;

	const double maxFrame = static_cast<double>(clip->getBuffer()->getFrameCount());
	if (position > maxFrame) 
		position = maxFrame;

	playbackPosition = position;
}
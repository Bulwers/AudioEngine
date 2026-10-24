#include "AudioSource.h"

#include <algorithm>
#include <cmath>

AudioSource::AudioSource(std::shared_ptr<AudioClip> clip) 
	: clip(std::move(clip)),
	  buffer(this->clip ? this->clip->getBuffer() : nullptr),
	  clipVolume(this->clip ? this->clip->getVolume() : 1.0f)
{
	if (this->clip) 
	{
		loop.store(this->clip->isLooping(), std::memory_order_relaxed);
		if (buffer)
		{
			playbackRangeEnd.store(static_cast<double>(buffer->getFrameCount()), std::memory_order_relaxed);
		}
	}
}

void AudioSource::play() 
{
	if (!clip || !buffer)
		return;
	pendingCommand.store(PendingCommand::Play, std::memory_order_release);
}

void AudioSource::pause() 
{
	pendingCommand.store(PendingCommand::Pause, std::memory_order_release);
}

void AudioSource::stop()
{
	pendingCommand.store(PendingCommand::Stop, std::memory_order_release);
}

void AudioSource::restart() 
{
	if (!clip || !buffer)
		return;
	pendingCommand.store(PendingCommand::Restart, std::memory_order_release);
}

void AudioSource::setVolume(float newVolume) 
{
	if (std::isfinite(newVolume))
		volume.store(newVolume, std::memory_order_relaxed);
}

void AudioSource::setPitch(float newPitch) 
{
	if (std::isfinite(newPitch) && newPitch > 0.0f)
		pitch.store(newPitch, std::memory_order_relaxed);
}

void AudioSource::setLooping(bool shouldLoop) 
{
	loop.store(shouldLoop, std::memory_order_relaxed);
}

double AudioSource::getPlaybackPosition() const
{
	if (!clip || !buffer)
		return 0;

	return playbackPosition.load(std::memory_order_acquire);
}

void AudioSource::setPlaybackPosition(double position) 
{
	if (!clip || !buffer || !std::isfinite(position))
		return;

	const double maxFrame = static_cast<double>(buffer->getFrameCount());
	const double clampedStart = std::clamp(playbackRangeStart.load(std::memory_order_relaxed), 0.0, maxFrame);
	const double clampedEnd = std::clamp(playbackRangeEnd.load(std::memory_order_relaxed), clampedStart, maxFrame);

	position = std::clamp(position, clampedStart, clampedEnd);

	requestedSeekPosition.store(position, std::memory_order_relaxed);
	pendingCommand.store(PendingCommand::Seek, std::memory_order_release);
}

void AudioSource::setPlaybackRange(double start, double end)
{
	if (!std::isfinite(start) || !std::isfinite(end))
		return;

	if (!clip || !buffer)
	{
		playbackRangeStart.store(0.0, std::memory_order_relaxed);
		playbackRangeEnd.store(0.0, std::memory_order_relaxed);
		requestedSeekPosition.store(0.0, std::memory_order_relaxed);
		pendingCommand.store(PendingCommand::Seek, std::memory_order_release);
		return;
	}

	const double maxFrame = static_cast<double>(buffer->getFrameCount());

	start = std::clamp(start, 0.0, maxFrame);
	end = std::clamp(end, start, maxFrame);

	const double previousStart = playbackRangeStart.load(std::memory_order_relaxed);
	if (start > previousStart)
	{
		playbackRangeEnd.store(end, std::memory_order_relaxed);
		playbackRangeStart.store(start, std::memory_order_relaxed);
	}
	else
	{
		playbackRangeStart.store(start, std::memory_order_relaxed);
		playbackRangeEnd.store(end, std::memory_order_relaxed);
	}
}

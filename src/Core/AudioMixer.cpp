#include "AudioMixer.h"

#include <cmath>

void AudioMixer::addSource(std::shared_ptr<AudioSource> source)
{
	sources.push_back(source);
}

void AudioMixer::removeSource(std::shared_ptr<AudioSource> source)
{
	sources.erase(std::remove(sources.begin(), sources.end(), source), sources.end());
}

void AudioMixer::mixAudio(float* outputBuffer, size_t numFrames, size_t numChannels)
{
	if (!outputBuffer || numFrames <= 0 || numChannels <= 0)
		return;

	std::fill(outputBuffer, outputBuffer + numFrames * numChannels, 0.0f);

	for (const auto& source : sources)
	{
		if (!source || !source->isPlaying())
			continue;

		const auto& clip = source->getClip();
		if (!clip)
			continue;

		const auto& buffer = clip->getBuffer();
		if (!buffer || buffer->getChannels() == 0)
			continue;

		const size_t bufferFrames = buffer->getFrameCount();
		if (bufferFrames == 0)
		{
			source->stop();
			continue;
		}

		const size_t bufferChannels = static_cast<size_t>(buffer->getChannels());
		const float volume = source->getVolume() * clip->getVolume();
		const double pitch = static_cast<double>(source->getPitch());

		if (pitch <= 0.0)
			continue;

		double playbackPosition = source->getPlaybackPosition();

		for (size_t frame = 0; frame < numFrames; ++frame)
		{
			size_t sourceFrame = static_cast<size_t>(playbackPosition);

			if (sourceFrame >= bufferFrames)
			{
				if (source->isLooping())
				{
					playbackPosition = std::fmod(playbackPosition, static_cast<double>(bufferFrames));
					sourceFrame = static_cast<size_t>(playbackPosition);
				}
				else
				{
					source->stop();
					break;
				}
			}

			const size_t frame0 = sourceFrame;
			const double fractional = playbackPosition - static_cast<double>(frame0);
			size_t frame1 = frame0 + 1;

			if (frame1 >= bufferFrames)
			{
				if (source->isLooping())
				{
					frame1 = 0;
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
				if (channel < bufferChannels)
				{
					const float sample0 = buffer->getSample(frame0, channel);
					const float sample1 = buffer->getSample(frame1, channel);
					sampleValue = sample0 + alpha * (sample1 - sample0);
				}
				outputBuffer[frame * numChannels + channel] += sampleValue * volume;
			}
			playbackPosition += pitch;
		}

		if (source->isPlaying())
		{
			source->setPlaybackPosition(playbackPosition);
		}
	}
}
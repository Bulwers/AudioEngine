#include "AudioDevice.h"

#include <algorithm>

AudioDevice::~AudioDevice()
{
	shutdown();
}

void AudioDevice::shutdown()
{
	if (stream)
	{
		SDL_DestroyAudioStream(stream);
		stream = nullptr;
	}
	audioMixer = nullptr;
	mixBuffer.clear();
}

bool AudioDevice::initialize(AudioMixer* mixer)
{
	if (!mixer)
		return false;
	
	if (isInitialized())
		return true;

	this->audioMixer = mixer;
	format = mixer->getFormat();

	mixBuffer.resize(static_cast<size_t>(bufferSize) * static_cast<size_t>(format.channels));

	SDL_AudioSpec spec{};
	spec.format = SDL_AUDIO_F32;
	spec.channels = static_cast<int>(format.channels);
	spec.freq = static_cast<int>(format.sampleRate);

	stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, &AudioDevice::audioCallback, this);

	if (!stream)
	{
		this->audioMixer = nullptr;
		mixBuffer.clear();
		return false;
	}

	if (!SDL_ResumeAudioStreamDevice(stream))
	{
		SDL_DestroyAudioStream(stream);
		stream = nullptr;
		this->audioMixer = nullptr;
		mixBuffer.clear();
		return false;
	}
	return true;
}

bool AudioDevice::isInitialized() const
{
	return stream != nullptr;
}

bool AudioDevice::refreshDevice()
{
	AudioMixer* currentMixer = audioMixer;

	if (!currentMixer)
		return false;

	shutdown();

	return initialize(currentMixer);
}

void AudioDevice::setBufferSize(int newBufferSize)
{
	if (newBufferSize <= 0 || isInitialized())
		return;
	bufferSize = newBufferSize;
}

int AudioDevice::getBufferSize() const
{
	return bufferSize;
}

void AudioDevice::audioCallback(void* userdata, SDL_AudioStream* stream, int additionalAmount, int totalAmount)
{
	AudioDevice* device = static_cast<AudioDevice*>(userdata);

	if (!device || !device->audioMixer)
		return;

	const std::size_t bytesPerFrame = sizeof(float) * static_cast<std::size_t>(device->format.channels);
	const std::size_t requestedFrameCount = static_cast<std::size_t>(additionalAmount) / bytesPerFrame;

	if (requestedFrameCount == 0)
		return;

	std::size_t remainingFrames = requestedFrameCount;
	while (remainingFrames > 0)
	{
		const std::size_t framesToMix = std::min(remainingFrames, static_cast<std::size_t>(device->bufferSize));

		device->audioMixer->mixAudio(device->mixBuffer.data(), framesToMix);

		SDL_PutAudioStreamData(stream, device->mixBuffer.data(), static_cast<int>(framesToMix * bytesPerFrame));
	
		remainingFrames -= framesToMix;
	}
}

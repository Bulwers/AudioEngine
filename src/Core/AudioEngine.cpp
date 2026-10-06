#include "AudioEngine.h"

AudioEngine::AudioEngine(TickSystem* tickSystem)
	: tickSystem(tickSystem)
{
}

AudioEngine::~AudioEngine()
{
	shutdown();
}

bool AudioEngine::initialize()
{
	if (initialized)
		return true;

	resourceManager.loadAudioBuffers();
	resourceManager.createAudioClips();

	if (!audioDevice.initialize(&audioMixer))
		return false;

	initialized = true;
	return true;
}

void AudioEngine::shutdown()
{
	if (!initialized) return;

	audioDevice.shutdown();
	sources.clear();
	initialized = false;
}

void AudioEngine::loadAudioAssets()
{
	resourceManager.loadAudioBuffers();
	resourceManager.createAudioClips();
}

std::shared_ptr<AudioSource> AudioEngine::createAudioSource(const std::string& clipname)
{
	auto clip = resourceManager.getAudioClip(clipname);

	if (!clip)
		return nullptr;

	auto source = std::make_shared<AudioSource>(clip);

	sources.push_back(source);
	audioMixer.addSource(source);
	
	return source;
}
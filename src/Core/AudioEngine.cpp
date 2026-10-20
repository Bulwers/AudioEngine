#include "AudioEngine.h"

#include <algorithm>

void AudioEngine::loadAudioAssets()
{
	resourceManager.loadAudioBuffers();
	resourceManager.loadAudioClips();
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

void AudioEngine::removeAudioSource(std::shared_ptr<AudioSource> source)
{
	if (!source)
		return;

	audioMixer.removeSource(source);
	auto it = std::find(sources.begin(), sources.end(), source);
	if (it != sources.end())
	{
		sources.erase(it);
	}
}

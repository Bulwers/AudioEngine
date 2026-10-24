#include "AudioEngine.h"

#include <algorithm>
#include <utility>

AudioEngine::AudioEngine(AudioFormat format, AudioResourceConfig resourceConfig)
	: resourceManager(std::move(resourceConfig)), audioMixer(format)
{
}

void AudioEngine::loadAudioAssets()
{
	std::lock_guard<std::mutex> lock(sourcesMutex);
	resourceManager.loadAudioBuffers();
	resourceManager.loadAudioClips();
}

void AudioEngine::collectRetiredSources()
{
	audioMixer.collectRetiredProcessorChains();
	std::shared_ptr<AudioSource> retired;
	while (audioMixer.popRetiredSource(retired))
		retired.reset();
}

std::vector<std::shared_ptr<AudioSource>> AudioEngine::getAudioSources()
{
	std::lock_guard<std::mutex> lock(sourcesMutex);
	collectRetiredSources();
	return sources;
}

std::shared_ptr<AudioSource> AudioEngine::createAudioSource(const std::string& clipname)
{
	std::lock_guard<std::mutex> lock(sourcesMutex);
	collectRetiredSources();

	auto clip = resourceManager.getAudioClip(clipname);

	if (!clip)
		return nullptr;

	auto source = std::make_shared<AudioSource>(clip);
	sources.push_back(source);
	if (!audioMixer.enqueueAddSource(source))
	{
		sources.pop_back();
		return nullptr;
	}
	
	return source;
}

bool AudioEngine::removeAudioSource(const std::shared_ptr<AudioSource>& source)
{
	if (!source)
		return false;
	std::lock_guard<std::mutex> lock(sourcesMutex);
	collectRetiredSources();

	auto it = std::find(sources.begin(), sources.end(), source);
	if (it == sources.end())
		return false;

	if (!audioMixer.enqueueRemoveSource(*it))
		return false;

	sources.erase(it);
	return true;
}

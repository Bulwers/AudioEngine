#pragma once

#include <functional>
#include <memory>

class MixerPanel
{
public:

	explicit MixerPanel(class AudioEngine& audioEngine, std::function<void(std::shared_ptr<class AudioSource>)> onSourceSelected);

	void render();

private:

	AudioEngine& audioEngine;
	std::function<void(std::shared_ptr<AudioSource>)> onSourceSelected;
	std::weak_ptr<AudioSource> selectedSource;

};
#pragma once

#include "AudioEngine.h"

class EditorUI
{
public:

	EditorUI(AudioEngine& engine);

	void render();

private:

	void renderResourcePanel();
	void renderSourcePanel();
	void renderMixerPanel();

	AudioEngine& audioEngine;

	std::shared_ptr<AudioSource> selectedSource;
};
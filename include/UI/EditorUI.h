#pragma once

#include "AudioEngine.h"
#include "ResourcePanel.h"
#include "AudioSourcesPanel.h"
#include "ITickable.h"

class EditorUI : public ITickable
{
public:

	EditorUI(AudioEngine& engine, TickSystem& tickSystem);

	void tick(float deltaTime) override;

	void render();

	void uppdateSelectedSource(std::shared_ptr<AudioSource> source);

private:

	void renderResourcePanel();
	void renderAudioSourcesPanel();

	AudioEngine& audioEngine;

	ResourcePanel resourcePanel;
    AudioSourcesPanel audioSourcesPanel;

	std::shared_ptr<AudioSource> selectedSource;
};

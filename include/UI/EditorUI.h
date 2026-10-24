#pragma once

#include "AudioEngine.h"
#include "ResourcePanel.h"
#include "AudioSourcesPanel.h"
#include "EffectsPanel.h"
#include "VisualizerPanel.h"
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
	void renderEffectsPanel();
	void renderVisualizerPanel();

	AudioEngine& audioEngine;

	ResourcePanel resourcePanel;
    AudioSourcesPanel audioSourcesPanel;
	EffectsPanel effectsPanel;
	VisualizerPanel visualizerPanel;

	std::shared_ptr<AudioSource> selectedSource;
};

#pragma once

#include "AudioEngine.h"
#include "ResourcePanel.h"
#include "SourcePanel.h"
#include "MixerPanel.h"
#include "ITickable.h"

class EditorUI : public ITickable
{
public:

	EditorUI(AudioEngine& engine);

	void tick(float deltaTime) override;

	void render();

	void uppdateSelectedSource(std::shared_ptr<AudioSource> source) { selectedSource = source; }

private:

	void renderResourcePanel();
	void renderSourcePanel();
	void renderMixerPanel();

	AudioEngine& audioEngine;

	ResourcePanel resourcePanel;
	SourcePanel sourcePanel;
	MixerPanel mixerPanel;

	std::shared_ptr<AudioSource> selectedSource;
};
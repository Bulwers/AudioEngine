#pragma once

#include "FilterEffects.h"

#include <memory>
#include <string>

class AudioMixer;

class EffectsPanel
{
public:
	explicit EffectsPanel(AudioMixer& mixer);
	void render();

	std::shared_ptr<ParametricEqualizer> getEqualizer() const { return equalizer; }
	bool isEqualizerEnabled() const { return equalizerEnabled; }
	void setEqualizerEnabled(bool enabled);

private:
	void rebuildMasterChain();
	float getFrequencySliderMaximum() const;

	AudioMixer& mixer;
	std::shared_ptr<LowPassFilter> lowPass = std::make_shared<LowPassFilter>();
	std::shared_ptr<HighPassFilter> highPass = std::make_shared<HighPassFilter>();
	std::shared_ptr<ParametricEqualizer> equalizer = std::make_shared<ParametricEqualizer>();

	bool lowPassEnabled = false;
	bool highPassEnabled = false;
	bool equalizerEnabled = false;
	bool chainRebuildPending = false;
	float lowPassCutoffHz = 1200.0f;
	float highPassCutoffHz = 80.0f;
	std::string statusMessage;
};

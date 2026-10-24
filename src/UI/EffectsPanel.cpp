#include "EffectsPanel.h"

#include "AudioMixer.h"
#include "AudioProcessorChain.h"
#include "imgui.h"

#include <algorithm>
#include <memory>

EffectsPanel::EffectsPanel(AudioMixer& mixer)
	: mixer(mixer)
{
}

void EffectsPanel::setEqualizerEnabled(bool enabled)
{
	if (equalizerEnabled == enabled)
		return;
	equalizerEnabled = enabled;
	rebuildMasterChain();
}

float EffectsPanel::getFrequencySliderMaximum() const
{
	return std::max(20.0f, std::min(20000.0f, static_cast<float>(mixer.getFormat().sampleRate) * 0.49f));
}

void EffectsPanel::rebuildMasterChain()
{
	chainRebuildPending = true;
	statusMessage.clear();

	auto chain = std::make_shared<AudioProcessorChain>();
	if (lowPassEnabled && !chain->addProcessor(lowPass))
		return;
	if (highPassEnabled && !chain->addProcessor(highPass))
		return;
	if (equalizerEnabled && !chain->addProcessor(equalizer))
		return;

	std::shared_ptr<AudioProcessorChain> requestedChain;
	if (lowPassEnabled || highPassEnabled || equalizerEnabled)
		requestedChain = std::move(chain);

	if (!mixer.setMasterProcessorChain(std::move(requestedChain)))
	{
		statusMessage = "DSP update queue is full. The change will be retried.";
		return;
	}

	chainRebuildPending = false;
}

void EffectsPanel::render()
{
	if (chainRebuildPending)
		rebuildMasterChain();

	ImGui::SetNextWindowSize(ImVec2(420.0f, 520.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("DSP Effects");

	if (ImGui::Checkbox("Enable Low-pass", &lowPassEnabled))
		rebuildMasterChain();
	if (lowPassEnabled)
	{
		const float maximum = getFrequencySliderMaximum();
		if (ImGui::SliderFloat("Low-pass cutoff", &lowPassCutoffHz, 20.0f, maximum, "%.0f Hz", ImGuiSliderFlags_Logarithmic))
			lowPass->setCutoffFrequency(lowPassCutoffHz);
	}

	if (ImGui::Checkbox("Enable High-pass", &highPassEnabled))
		rebuildMasterChain();
	if (highPassEnabled)
	{
		const float maximum = getFrequencySliderMaximum();
		if (ImGui::SliderFloat("High-pass cutoff", &highPassCutoffHz, 20.0f, maximum, "%.0f Hz", ImGuiSliderFlags_Logarithmic))
			highPass->setCutoffFrequency(highPassCutoffHz);
	}

	if (ImGui::Checkbox("Enable parametric EQ", &equalizerEnabled))
		rebuildMasterChain();

	if (!statusMessage.empty())
		ImGui::TextWrapped("%s", statusMessage.c_str());

	ImGui::End();
}

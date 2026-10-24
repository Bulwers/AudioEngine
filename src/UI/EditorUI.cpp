#include "EditorUI.h"
#include "imgui.h"

EditorUI::EditorUI(AudioEngine& engine, TickSystem& tickSystem)
	: audioEngine(engine), ITickable(tickSystem), 
	resourcePanel(audioEngine.getResourceManager(), [this](const std::string& clipName)
		{
			uppdateSelectedSource(audioEngine.createAudioSource(clipName));
       }),
	audioSourcesPanel(audioEngine),
	effectsPanel(audioEngine.getAudioMixer()),
	visualizerPanel(audioEngine.getAudioMixer(), effectsPanel)
{
}

void EditorUI::tick(float deltaTime)
{
	render();
}

void EditorUI::render()
{
	ImGui::DockSpaceOverViewport();
	
	renderResourcePanel();
    renderAudioSourcesPanel();
	renderEffectsPanel();
	renderVisualizerPanel();
}

void EditorUI::uppdateSelectedSource(std::shared_ptr<AudioSource> source)
{
	selectedSource = std::move(source);
	audioSourcesPanel.setSelectedSource(selectedSource);
}

void EditorUI::renderResourcePanel()
{
	resourcePanel.render();
}

void EditorUI::renderAudioSourcesPanel()
{
    audioSourcesPanel.render();
}

void EditorUI::renderEffectsPanel()
{
	effectsPanel.render();
}

void EditorUI::renderVisualizerPanel()
{
	visualizerPanel.render();
}

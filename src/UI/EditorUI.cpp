#include "EditorUI.h"
#include "imgui.h"

EditorUI::EditorUI(AudioEngine& engine, TickSystem& tickSystem)
	: audioEngine(engine), ITickable(tickSystem), 
	resourcePanel(audioEngine.getResourceManager(), [this](const std::string& clipName)
		{
			uppdateSelectedSource(audioEngine.createAudioSource(clipName));
       }),
	audioSourcesPanel(audioEngine)
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

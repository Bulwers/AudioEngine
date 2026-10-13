#include "EditorUI.h"
#include "EditorUI.h"
#include "imgui.h"

EditorUI::EditorUI(AudioEngine& engine)
	: audioEngine(engine), ITickable(engine.getTickSystem()), 
	resourcePanel(audioEngine.getResourceManager(), [this](const std::string& clipName)
		{
			uppdateSelectedSource(audioEngine.createAudioSource(clipName));
      }),
	mixerPanel(audioEngine, [this](std::shared_ptr<AudioSource> source)
		{
			uppdateSelectedSource(std::move(source));
		})
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
	renderSourcePanel();
	renderMixerPanel();
}

void EditorUI::uppdateSelectedSource(std::shared_ptr<AudioSource> source)
{
	selectedSource = std::move(source);
	sourcePanel.setSelectedSource(selectedSource);
}

void EditorUI::renderResourcePanel()
{
	resourcePanel.render();
}

void EditorUI::renderSourcePanel()
{
	sourcePanel.render();
}

void EditorUI::renderMixerPanel()
{
	mixerPanel.render();
}
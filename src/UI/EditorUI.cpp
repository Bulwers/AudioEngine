#include "EditorUI.h"
#include "imgui.h"

EditorUI::EditorUI(AudioEngine& engine)
	: audioEngine(engine), ITickable(engine.getTickSystem()), 
	resourcePanel(audioEngine.getResourceManager(), [this](const std::string& clipName)
		{
			uppdateSelectedSource(audioEngine.createAudioSource(clipName));
		})
{
	uppdateSelectedSource(audioEngine.createAudioSource("Test1"));
}

void EditorUI::tick(float deltaTime)
{
	render();
}

void EditorUI::render()
{
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
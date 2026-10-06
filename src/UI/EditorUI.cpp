#include "EditorUI.h"
#include "imgui.h"

EditorUI::EditorUI(AudioEngine& engine)
	: audioEngine(engine), ITickable(engine.getTickSystem()), resourcePanel(audioEngine.getResourceManager())
{
	selectedSource = audioEngine.createAudioSource("Test1");
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
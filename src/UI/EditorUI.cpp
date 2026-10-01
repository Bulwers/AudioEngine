#include "EditorUI.h"
#include "imgui.h"

EditorUI::EditorUI(AudioEngine& engine)
	: audioEngine(engine)
{
	selectedSource = audioEngine.createAudioSource("Test1");
}

void EditorUI::render()
{
	//renderResourcePanel();
	renderSourcePanel();
	//renderMixerPanel();
}

void EditorUI::renderResourcePanel()
{
	// Render the resource panel UI here
}

void EditorUI::renderSourcePanel()
{
	ImGui::Begin("Source Panel");

	if (!selectedSource)
	{
		ImGui::Text("No source selected.");
		ImGui::End();
		return;
	}

	if (ImGui::Button("Play"))
	{
		selectedSource->play();
	}

	ImGui::SameLine();

	if (ImGui::Button("Pause"))
	{
		selectedSource->pause();
	}

	ImGui::SameLine();

	if (ImGui::Button("Stop"))
	{
		selectedSource->stop();
	}

	float volume = selectedSource->getVolume();

	if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f))
	{
		selectedSource->setVolume(volume);
	}

	float pitch = selectedSource->getPitch();

	if (ImGui::SliderFloat("Pitch", &pitch, 0.5f, 2.0f))
	{
		selectedSource->setPitch(pitch);
	}

	bool loop = selectedSource->isLooping();
	
	if (ImGui::Checkbox("Loop", &loop))
	{
		selectedSource->setLooping(loop);
	}

	ImGui::Text("Playback Position: %.2f seconds", selectedSource->getPlaybackPosition());

	ImGui::End();
}

void EditorUI::renderMixerPanel()
{
	// Render the mixer panel UI here
}
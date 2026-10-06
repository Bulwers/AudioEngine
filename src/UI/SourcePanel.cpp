#include "SourcePanel.h"
#include "AudioSource.h"
#include "imgui.h"

void SourcePanel::render()
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
	ImGui::End();
}
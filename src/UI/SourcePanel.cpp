#include "SourcePanel.h"
#include "AudioSource.h"
#include "imgui.h"

#include <algorithm>

void SourcePanel::render()
{
	ImGui::SetNextWindowSize(ImVec2(420.0f, 200.0f), ImGuiCond_Appearing);
	ImGui::Begin("Source Panel");

    auto source = selectedSource.lock();
	if (!source)
	{
		ImGui::Text("No source selected.");
		ImGui::End();
		return;
	}

	if (ImGui::Button("Play"))
	{
		source->play();
	}

	ImGui::SameLine();
	if (ImGui::Button("Pause"))
	{
        source->pause();
	}
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
	{
		source->stop();
	}

	float volume = source->getVolume();
	if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f))
	{
		source->setVolume(volume);
	}
	if (ImGui::InputFloat("Volume Value", &volume, 0.01f, 0.1f, "%.3f"))
	{
		source->setVolume(std::clamp(volume, 0.0f, 1.0f));
	}

	float pitch = source->getPitch();
	if (ImGui::SliderFloat("Pitch", &pitch, 0.5f, 2.0f))
	{
        source->setPitch(pitch);
	}
   if (ImGui::InputFloat("Pitch Value", &pitch, 0.01f, 0.1f, "%.3f"))
	{
		source->setPitch(std::clamp(pitch, 0.5f, 2.0f));
	}
	ImGui::End();
}
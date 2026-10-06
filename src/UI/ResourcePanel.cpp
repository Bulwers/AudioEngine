#include "ResourcePanel.h"
#include "ResourceManager.h"
#include "imgui.h"

ResourcePanel::ResourcePanel(ResourceManager& resourceManager)
	: resourceManager(resourceManager)
{
}

void ResourcePanel::render()
{
	ImGui::Begin("Resource Panel");

	static std::string statusMessage = "Ready.";

	if (ImGui::Button("Load Audio Buffers"))
	{
		resourceManager.loadAudioBuffers();
		statusMessage = "Audio buffers loaded.";
	}
	ImGui::SameLine();
	if (ImGui::Button("Create Clips"))
	{
		resourceManager.createAudioClips();
		statusMessage = "Audio clips created.";
	}
	ImGui::SameLine();
	if (ImGui::Button("Save Clips"))
	{
		resourceManager.saveAudioClips();
		statusMessage = "Audio clips saved. Assets/AudioClips";
	}
	ImGui::Separator();
	ImGui::TextWrapped("Status: %s", statusMessage.c_str());
	
	ImGui::End();
}
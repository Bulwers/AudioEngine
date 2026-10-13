#include "ResourcePanel.h"
#include "ResourceManager.h"
#include "imgui.h"

namespace
{
	const char* audioCategoryToSting(AudioCategory category)
	{
		switch (category)
		{
		case AudioCategory::Ambient: return "Ambient";
		case AudioCategory::Music: return "Music";
		case AudioCategory::Other: return "Other";
		case AudioCategory::SFX: return "SFX";
		case AudioCategory::UI: return "UI";
		case AudioCategory::Voice: return "Voice";
		default: return "Unknown";
		}
	}
}

ResourcePanel::ResourcePanel(ResourceManager& resourceManager, std::function<void(const std::string&)> onSourceCreated)
	: resourceManager(resourceManager), onSourceCreated(std::move(onSourceCreated))
{
}

void ResourcePanel::render()
{
	ImGui::SetNextWindowSize(ImVec2(420.0f, 450.0f), ImGuiCond_Appearing);
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

	const auto& clips = resourceManager.getAudioClips();

	ImGui::Text("Available Audio Clips:");
	if (clips.empty())
	{
		selectedClipName.clear();
		ImGui::TextWrapped("No clips available. Please load audio buffers and create clips.");
	}
	else
	{
		if (ImGui::BeginListBox("##ClipList", ImVec2(-FLT_MIN, 200.0f)))
		{
			for (const auto& pair : clips)
			{
				const std::string& clipName = pair.first;
				bool isSelected = (selectedClipName == clipName);

				if (ImGui::Selectable(clipName.c_str(), isSelected))
				{
					selectedClipName = clipName;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndListBox();
		}
	}

	std::shared_ptr<AudioClip> selectedClip;
	if (!selectedClipName.empty())
	{
		selectedClip = resourceManager.getAudioClip(selectedClipName);
		if (!selectedClip)
		{
			selectedClipName.clear();
		}
	}

	if (selectedClip)
	{
		ImGui::Separator();
		ImGui::Text("Selected Clip Info:");
		ImGui::TextWrapped("Name: %s", selectedClip->getName().c_str());
		ImGui::TextWrapped("File Path: %s", selectedClip->getFilePath().c_str());
		ImGui::Text("Duration: %.2f seconds", selectedClip->getDuration());
		ImGui::Text("Category: %s", audioCategoryToSting(selectedClip->getCategory()));
		ImGui::Text("Volume: %.2f", selectedClip->getVolume());
		ImGui::Text("Looping: %s", selectedClip->isLooping() ? "Yes" : "No");

		const bool canCreateSource = static_cast<bool>(onSourceCreated);
		if (!canCreateSource)
		{
			ImGui::BeginDisabled();
		}

		if (ImGui::Button("Create Audio Source"))
		{
			onSourceCreated(selectedClip->getName());
			statusMessage = "Audio source created for clip: " + selectedClip->getName();
		}
		
		if (!canCreateSource)
		{
			ImGui::EndDisabled();
		}
	}

	ImGui::Separator();
	ImGui::TextWrapped("Status: %s", statusMessage.c_str());
	
	ImGui::End();
}
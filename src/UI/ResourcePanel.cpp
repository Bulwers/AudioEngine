#include "ResourcePanel.h"
#include "ResourcePanel.h"
#include "ResourceManager.h"
#include "imgui.h"

#include <cstring>

namespace
{
	const char* audioCategoryToString(AudioCategory category)
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

	AudioCategory indexToCategory(int index)
	{
		switch (index)
		{
		case 0: return AudioCategory::Ambient;
		case 1: return AudioCategory::Music;
		case 2: return AudioCategory::SFX;
		case 3: return AudioCategory::UI;
		case 4: return AudioCategory::Voice;
		default: return AudioCategory::Other;
		}
	}
}

ResourcePanel::ResourcePanel(ResourceManager& resourceManager, std::function<void(const std::string&)> onSourceCreated)
	: resourceManager(resourceManager), onSourceCreated(std::move(onSourceCreated))
{
}

void ResourcePanel::render()
{
	ImGui::SetNextWindowSize(ImVec2(460.0f, 520.0f), ImGuiCond_Appearing);
	ImGui::Begin("Resource Panel");

	static std::string statusMessage = "Ready.";

	if (!wavFilesInitialized)
	{
		availableWavFiles = resourceManager.getAvailableWavFiles();
		selectedWavFileIndex = availableWavFiles.empty() ? -1 : 0;
		wavFilesInitialized = true;
	}
	
	const char* selectedWavLabel = "No .wav files found";
	const auto& resourceConfig = resourceManager.getConfig();
	const std::string sourceDirectory = (resourceConfig.rootPath / resourceConfig.sourceDirectory).generic_string();
	const std::string clipDirectory = (resourceConfig.rootPath / resourceConfig.clipDirectory).generic_string();
	if (!availableWavFiles.empty() && selectedWavFileIndex >= 0)
	{
		selectedWavLabel = availableWavFiles[static_cast<size_t>(selectedWavFileIndex)].c_str();
	}

	if (ImGui::BeginCombo("WAV file", selectedWavLabel))
	{
		if (availableWavFiles.empty())
		{
			ImGui::BeginDisabled();
			const std::string emptyMessage = "No .wav files found in " + sourceDirectory;
			ImGui::Selectable(emptyMessage.c_str(), false);
			ImGui::EndDisabled();
		}
		else
		{
			for (int i = 0; i < static_cast<int>(availableWavFiles.size()); ++i)
			{
				const bool isSelected = selectedWavFileIndex == i;
				if (ImGui::Selectable(availableWavFiles[static_cast<size_t>(i)].c_str(), isSelected))
				{
					selectedWavFileIndex = i;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}
			}
		}
		ImGui::EndCombo();
	}

	ImGui::InputText("Clip Name", newClipName, sizeof(newClipName));

	const char* categories[] = { "Ambient", "Music", "SFX", "UI", "Voice", "Other" };
	ImGui::Combo("Category", &selectedCategoryIndex, categories, IM_ARRAYSIZE(categories));

	if (ImGui::Button("Create Clip"))
	{
		if (selectedWavFileIndex < 0 || selectedWavFileIndex >= static_cast<int>(availableWavFiles.size()))
		{
			statusMessage = "Select a .wav file first.";
		}
		else
		{
			std::string errorMessage;
			const bool created = resourceManager.createAndSaveAudioClip(
			availableWavFiles[static_cast<size_t>(selectedWavFileIndex)],
			newClipName,
			indexToCategory(selectedCategoryIndex),
			errorMessage);

			if (created)
			{
				selectedClipName = newClipName;
				statusMessage = "Clip saved to " + clipDirectory + "/" + categories[selectedCategoryIndex] + "/";
				std::memset(newClipName, 0, sizeof(newClipName));
			}
			else
			{
				statusMessage = errorMessage.empty() ? "Failed to save clip." : errorMessage;
			}
		}
	}
	ImGui::SameLine();
	if (ImGui::Button("Load Clips"))
	{
		resourceManager.loadAudioClips();
		statusMessage = "Loaded clips.";
	}

	ImGui::Separator();

	const auto& clips = resourceManager.getAudioClips();
	const bool canCreateSource = static_cast<bool>(onSourceCreated);
	std::string clipToRemove;

	ImGui::Text("Available Audio Clips:");
	if (clips.empty())
	{
		selectedClipName.clear();
		ImGui::TextWrapped("No clips available.");
	}
	else
	{
		if (ImGui::BeginListBox("##ClipList", ImVec2(-FLT_MIN, 200.0f)))
		{
			for (const auto& pair : clips)
			{
				const std::string& clipName = pair.first;
				const bool isSelected = (selectedClipName == clipName);
				ImGui::PushID(clipName.c_str());

                const float createButtonWidth = ImGui::CalcTextSize("Create").x + ImGui::GetStyle().FramePadding.x * 2.0f;
				const float removeButtonWidth = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2.0f;
				const float buttonWidth = createButtonWidth + ImGui::GetStyle().ItemSpacing.x + removeButtonWidth;
				const float rowWidth = ImGui::GetContentRegionAvail().x;
				const float selectableWidth = rowWidth > buttonWidth ? rowWidth - buttonWidth - ImGui::GetStyle().ItemSpacing.x : rowWidth;

				if (ImGui::Selectable(clipName.c_str(), isSelected, 0, ImVec2(selectableWidth, 0.0f)))
				{
					selectedClipName = clipName;
				}
				if (isSelected)
				{
					ImGui::SetItemDefaultFocus();
				}

				ImGui::SameLine();
				if (!canCreateSource)
				{
					ImGui::BeginDisabled();
				}

				if (ImGui::SmallButton("Create"))
				{
					onSourceCreated(clipName);
					statusMessage = "Audio source created for clip: " + clipName;
				}

				if (!canCreateSource)
				{
					ImGui::EndDisabled();
				}

				ImGui::SameLine();
				if (ImGui::SmallButton("X"))
				{
					clipToRemove = clipName;
				}

				ImGui::PopID();
			}
			ImGui::EndListBox();
		}
	}

	if (!clipToRemove.empty())
	{
		std::string errorMessage;
		if (resourceManager.removeAudioClipFile(clipToRemove, errorMessage))
		{
			if (selectedClipName == clipToRemove)
			{
				selectedClipName.clear();
			}
			statusMessage = "Removed clip file for: " + clipToRemove;
		}
		else
		{
			statusMessage = errorMessage.empty() ? "Failed to remove clip file." : errorMessage;
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
		ImGui::Text("Category: %s", audioCategoryToString(selectedClip->getCategory()));
		ImGui::Text("Volume: %.2f", selectedClip->getVolume());
		ImGui::Text("Looping: %s", selectedClip->isLooping() ? "Yes" : "No");
	}

	ImGui::Separator();
	ImGui::TextWrapped("Status: %s", statusMessage.c_str());

	ImGui::End();
}

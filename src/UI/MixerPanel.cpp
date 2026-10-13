#include "MixerPanel.h"
#include "AudioEngine.h"
#include "AudioSource.h"
#include "imgui.h"

#include <string>
#include <utility>

namespace
{
	const char* playbackStateToString(PlaybackState state)
	{
		switch (state)
		{
		case PlaybackState::Stopped: return "Stopped";
		case PlaybackState::Playing: return "Playing";
		case PlaybackState::Paused: return "Paused";
		default: return "Unknown";
		}
	}
}

MixerPanel::MixerPanel(AudioEngine& audioEngine, std::function<void(std::shared_ptr<AudioSource>)> onSourceSelected)
	: audioEngine(audioEngine), onSourceSelected(std::move(onSourceSelected))
{
}

void MixerPanel::render()
{
    ImGui::SetNextWindowSize(ImVec2(420.0f, 320.0f), ImGuiCond_Appearing);
	ImGui::Begin("Mixer Panel");

	auto& sources = audioEngine.getAudioSources();

	if (sources.empty())
	{
		selectedSource.reset();
		ImGui::TextWrapped("No audio sources created yet.");
		ImGui::End();
		return;
	}

	ImGui::Text("Created Audio Sources:");
	ImGui::Separator();

	std::shared_ptr<AudioSource> sourceToDelete;

	for (size_t i = 0; i < sources.size(); ++i)
	{
		const auto& source = sources[i];
		if (!source)
		{
			continue;
		}

		ImGui::PushID(static_cast<int>(i));

		const auto selected = selectedSource.lock();
		const bool isSelected = selected && selected == source;

		std::string clipName = "Unknown";
		if (source->getClip())
		{
			clipName = source->getClip()->getName();
		}

		std::string sourceLabel = clipName + "##source_select";
		if (ImGui::Selectable(sourceLabel.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns))
		{
			selectedSource = source;
			if (onSourceSelected)
			{
				onSourceSelected(source);
			}
		}

		ImGui::SameLine();
		ImGui::TextDisabled("[%s]", playbackStateToString(source->getState()));

		if (ImGui::SmallButton("Play"))
		{
			source->play();
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Pause"))
		{
			source->pause();
		}
		ImGui::SameLine();
		if (ImGui::SmallButton("Stop"))
		{
			source->stop();
		}
		ImGui::SameLine();

		bool isLooping = source->isLooping();
		if (ImGui::Checkbox("Loop", &isLooping))
		{
			source->setLooping(isLooping);
		}

		const float deleteButtonWidth = ImGui::CalcTextSize("Delete").x + ImGui::GetStyle().FramePadding.x * 2.0f;
		const float rightAlignedDeleteX = ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - deleteButtonWidth;
		ImGui::SameLine();
		if (rightAlignedDeleteX > ImGui::GetCursorPosX())
		{
			ImGui::SetCursorPosX(rightAlignedDeleteX);
		}
		if (ImGui::SmallButton("Delete"))
		{
			sourceToDelete = source;
		}

		ImGui::Separator();
		ImGui::PopID();
	}

	if (sourceToDelete)
	{
		if (selectedSource.lock() == sourceToDelete)
		{
			selectedSource.reset();
			if (onSourceSelected)
			{
				onSourceSelected(nullptr);
			}
		}

		audioEngine.removeAudioSource(sourceToDelete);
	}

	ImGui::End();
}
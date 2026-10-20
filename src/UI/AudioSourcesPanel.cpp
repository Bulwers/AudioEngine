#include "AudioSourcesPanel.h"

#include "AudioEngine.h"
#include "AudioSource.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <string>

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

AudioSourcesPanel::AudioSourcesPanel(AudioEngine& audioEngine)
    : audioEngine(audioEngine)
{
}

void AudioSourcesPanel::render()
{
    ImGui::SetNextWindowSize(ImVec2(460.0f, 420.0f), ImGuiCond_Appearing);
    ImGui::Begin("Audio Sources Panel");

    auto& sources = audioEngine.getAudioSources();
    std::shared_ptr<AudioSource> sourceToDelete;

    if (sources.empty())
    {
        selectedSource.reset();
        ImGui::TextWrapped("No audio sources created.");
        ImGui::End();
        return;
    }

    ImGui::Text("Created Audio Sources:");
    ImGui::Separator();

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

        std::string sourceLabel = clipName + " [" + playbackStateToString(source->getState()) + "]##source_select";
        const float buttonWidth = ImGui::CalcTextSize("X").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        const float rowWidth = ImGui::GetContentRegionAvail().x;
        const float selectableWidth = rowWidth > buttonWidth ? rowWidth - buttonWidth - ImGui::GetStyle().ItemSpacing.x : rowWidth;

        if (ImGui::Selectable(sourceLabel.c_str(), isSelected, 0, ImVec2(selectableWidth, 0.0f)))
        {
            selectedSource = source;
        }

        ImGui::SameLine();
        if (ImGui::SmallButton("X"))
        {
            sourceToDelete = source;
        }

        ImGui::PopID();
    }

    if (sourceToDelete)
    {
        if (selectedSource.lock() == sourceToDelete)
        {
            selectedSource.reset();
        }
        audioEngine.removeAudioSource(sourceToDelete);
    }

    auto source = selectedSource.lock();
    if (!source)
    {
        ImGui::End();
        return;
    }

    ImGui::Separator();
    ImGui::Text("Selected Source Controls");

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

    bool isLooping = source->isLooping();
    if (ImGui::Checkbox("Loop", &isLooping))
    {
        source->setLooping(isLooping);
    }

    float volume = source->getVolume();
    if (ImGui::SliderFloat("Volume", &volume, 0.0f, 1.0f))
    {
        source->setVolume(volume);
    }

    float pitch = source->getPitch();
    if (ImGui::SliderFloat("Pitch", &pitch, 0.5f, 2.0f))
    {
        source->setPitch(pitch);
    }

    const auto clip = source->getClip();
    const auto buffer = clip ? clip->getBuffer() : nullptr;
    if (!buffer || buffer->getSampleRate() == 0)
    {
        ImGui::TextWrapped("No valid audio clip assigned to this source.");
        ImGui::End();
        return;
    }

    const float sampleRate = static_cast<float>(buffer->getSampleRate());
    const float durationSeconds = static_cast<float>(buffer->getDuration());
    const float playbackPositionSeconds = static_cast<float>(source->getPlaybackPosition() / sampleRate);
    const float rangeStartSeconds = static_cast<float>(source->getPlaybackRangeStart() / sampleRate);
    const float rangeEndSeconds = static_cast<float>(source->getPlaybackRangeEnd() / sampleRate);

    ImGui::Text("Playback Position: %.3f s / %.3f s", playbackPositionSeconds, durationSeconds);
    ImGui::Text("Playback Range: %.3f s - %.3f s", rangeStartSeconds, rangeEndSeconds);

    if (durationSeconds > 0.0f)
    {
        float clipBeginSeconds = rangeStartSeconds;
        if (ImGui::SliderFloat("Clip Begin", &clipBeginSeconds, 0.0f, durationSeconds, "%.3f s"))
        {
            const double beginFrame = static_cast<double>(clipBeginSeconds * sampleRate);
            source->setPlaybackRange(beginFrame, source->getPlaybackRangeEnd());
        }

        float updatedBeginSeconds = static_cast<float>(source->getPlaybackRangeStart() / sampleRate);
        float clipEndSeconds = static_cast<float>(source->getPlaybackRangeEnd() / sampleRate);
        if (ImGui::SliderFloat("Clip End", &clipEndSeconds, updatedBeginSeconds, durationSeconds, "%.3f s"))
        {
            const double beginFrame = static_cast<double>(updatedBeginSeconds * sampleRate);
            const double endFrame = static_cast<double>(clipEndSeconds * sampleRate);
            source->setPlaybackRange(beginFrame, endFrame);
        }

        float playheadSeconds = static_cast<float>(source->getPlaybackPosition() / sampleRate);
        float minPlayhead = static_cast<float>(source->getPlaybackRangeStart() / sampleRate);
        float maxPlayhead = static_cast<float>(source->getPlaybackRangeEnd() / sampleRate);
        if (maxPlayhead < minPlayhead)
        {
            maxPlayhead = minPlayhead;
        }

        if (ImGui::SliderFloat("Playback Position", &playheadSeconds, minPlayhead, maxPlayhead, "%.3f s"))
        {
            source->setPlaybackPosition(static_cast<double>(playheadSeconds * sampleRate));
        }
    }

    ImGui::End();
}

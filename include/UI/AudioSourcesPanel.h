#pragma once

#include <memory>

class AudioEngine;
class AudioSource;

class AudioSourcesPanel
{
public:

    explicit AudioSourcesPanel(AudioEngine& audioEngine);

    void render();
    void setSelectedSource(std::shared_ptr<AudioSource> source) { selectedSource = source; }

private:

    AudioEngine& audioEngine;
    std::weak_ptr<AudioSource> selectedSource;
};

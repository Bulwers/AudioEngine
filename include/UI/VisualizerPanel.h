#pragma once

#include <array>
#include <complex>
#include <cstddef>
#include <vector>

class AudioMixer;
class EffectsPanel;

class VisualizerPanel
{
public:
	VisualizerPanel(AudioMixer& mixer, EffectsPanel& effectsPanel);
	void render();

private:
	static constexpr std::size_t FftSize = 1024;
	static constexpr std::size_t SpectrumBinCount = 64;

	void updateAudioSnapshot();
	void drawWaveform();
	void drawSpectrum();
	void drawEqualizerResponse();
	void calculateSpectrum();
	float frequencyToX(float frequency, float minFrequency, float maxFrequency,
		float left, float width) const;
	float xToFrequency(float x, float minFrequency, float maxFrequency,
		float left, float width) const;
	float gainToY(float gainDb, float top, float height) const;
	float yToGain(float y, float top, float height) const;

	AudioMixer& mixer;
	EffectsPanel& effectsPanel;
	std::vector<float> audioSamples;
	std::size_t audioSampleCount = 0;
	std::array<std::complex<float>, FftSize> fftData{};
	std::array<float, SpectrumBinCount> spectrumDb{};
	float peakLevel = 0.0f;
	float rmsLevel = 0.0f;
	int draggedEqualizerBand = -1;
	int selectedEqualizerBand = 0;
};

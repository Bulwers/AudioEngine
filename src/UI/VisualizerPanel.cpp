#include "VisualizerPanel.h"

#include "AudioMixer.h"
#include "EffectsPanel.h"
#include "FilterEffects.h"
#include "imgui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <string>

namespace
{
	constexpr float MinFrequencyHz = 20.0f;
	constexpr float MinGainDb = -24.0f;
	constexpr float MaxGainDb = 24.0f;
	constexpr float Pi = 3.14159265358979323846f;

	ImU32 color(unsigned int r, unsigned int g, unsigned int b, unsigned int a = 255)
	{
		return IM_COL32(r, g, b, a);
	}

	std::string formatFrequency(float frequency)
	{
		if (frequency >= 1000.0f)
		{
			const float kiloHertz = frequency / 1000.0f;
			return kiloHertz == std::floor(kiloHertz)
				? std::to_string(static_cast<int>(kiloHertz)) + "k"
				: std::to_string(kiloHertz).substr(0, 3) + "k";
		}
		return std::to_string(static_cast<int>(std::round(frequency)));
	}
}

VisualizerPanel::VisualizerPanel(AudioMixer& mixer, EffectsPanel& effectsPanel)
	: mixer(mixer), effectsPanel(effectsPanel), audioSamples(AudioMixer::VisualizationCapacity, 0.0f)
{
	spectrumDb.fill(-72.0f);
}

void VisualizerPanel::updateAudioSnapshot()
{
	const std::size_t copied = mixer.copyVisualizationSamples(audioSamples.data(), audioSamples.size());
	if (copied == 0)
		return;

	audioSampleCount = copied;
	peakLevel = 0.0f;
	double squareSum = 0.0;
	for (std::size_t i = 0; i < audioSampleCount; ++i)
	{
		const float sample = audioSamples[i];
		peakLevel = std::max(peakLevel, std::abs(sample));
		squareSum += static_cast<double>(sample) * sample;
	}
	rmsLevel = static_cast<float>(std::sqrt(squareSum / static_cast<double>(audioSampleCount)));
	calculateSpectrum();
}

void VisualizerPanel::render()
{
	updateAudioSnapshot();
	ImGui::SetNextWindowSize(ImVec2(620.0f, 780.0f), ImGuiCond_FirstUseEver);
	ImGui::Begin("Audio Visualizer");
	ImGui::Text("Output  |  Peak: %.3f  RMS: %.3f", peakLevel, rmsLevel);
	drawWaveform();
	drawSpectrum();

	bool equalizerEnabled = effectsPanel.isEqualizerEnabled();
	if (ImGui::Checkbox("Enable 3-band EQ", &equalizerEnabled))
		effectsPanel.setEqualizerEnabled(equalizerEnabled);
	drawEqualizerResponse();
	ImGui::End();
}

void VisualizerPanel::drawWaveform()
{
	ImGui::TextUnformatted("Waveform (latest output)");
	const ImVec2 size(std::max(100.0f, ImGui::GetContentRegionAvail().x), 130.0f);
	const ImVec2 topLeft = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##WaveformCanvas", size);
	const ImVec2 bottomRight(topLeft.x + size.x, topLeft.y + size.y);
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(topLeft, bottomRight, color(18, 21, 27));
	const float graphLeft = topLeft.x + 38.0f;
	const float graphTop = topLeft.y + 7.0f;
	const float graphWidth = size.x - 46.0f;
	const float graphHeight = size.y - 27.0f;

	const std::size_t channels = mixer.getFormat().channels;
	const std::size_t frameCount = channels == 0 ? 0 : audioSampleCount / channels;
	for (int tick = -2; tick <= 2; ++tick)
	{
		const float amplitude = static_cast<float>(tick) * 0.5f;
		const float y = graphTop + (1.0f - amplitude) * 0.5f * graphHeight;
		const bool major = tick == 0;
		drawList->AddLine(ImVec2(graphLeft, y), ImVec2(graphLeft + graphWidth, y),
			major ? color(95, 102, 112) : color(52, 57, 65));
		char label[16];
		std::snprintf(label, sizeof(label), "%+.1f", amplitude);
		drawList->AddText(ImVec2(topLeft.x + 2.0f, y - 7.0f), color(180, 184, 192), label);
	}
	for (int tick = 0; tick <= 4; ++tick)
	{
		const float x = graphLeft + graphWidth * static_cast<float>(tick) / 4.0f;
		drawList->AddLine(ImVec2(x, graphTop), ImVec2(x, graphTop + graphHeight), color(42, 47, 55));
	}
	if (frameCount < 2)
	{
		drawList->AddText(ImVec2(graphLeft + 8.0f, graphTop + 8.0f), color(180, 184, 192), "Waiting for audio output");
		return;
	}

	const std::size_t pixelColumns = std::max<std::size_t>(1, static_cast<std::size_t>(graphWidth));
	const std::size_t plottedChannels = std::min<std::size_t>(channels, 2);
	for (std::size_t channel = 0; channel < plottedChannels; ++channel)
	{
		const ImU32 waveformColor = channel == 0 ? color(86, 190, 255) : color(255, 177, 75);
		for (std::size_t column = 0; column < pixelColumns; ++column)
		{
			const std::size_t frameStart = column * frameCount / pixelColumns;
			const std::size_t frameEnd = std::max(frameStart + 1, (column + 1) * frameCount / pixelColumns);
			float minimum = 1.0f;
			float maximum = -1.0f;
			for (std::size_t frame = frameStart; frame < std::min(frameEnd, frameCount); ++frame)
			{
				const float sample = audioSamples[frame * channels + channel];
				minimum = std::min(minimum, sample);
				maximum = std::max(maximum, sample);
			}
			const float x = graphLeft + static_cast<float>(column);
			const float yMaximum = graphTop + (1.0f - std::clamp(maximum, -1.0f, 1.0f)) * 0.5f * graphHeight;
			const float yMinimum = graphTop + (1.0f - std::clamp(minimum, -1.0f, 1.0f)) * 0.5f * graphHeight;
			drawList->AddLine(ImVec2(x, yMaximum), ImVec2(x, yMinimum), waveformColor);
		}
	}
	const float sampleRate = std::max(1.0f, static_cast<float>(mixer.getFormat().sampleRate));
	const float durationSeconds = static_cast<float>(frameCount) / sampleRate;
	for (int tick = 0; tick <= 4; ++tick)
	{
		const float x = graphLeft + graphWidth * static_cast<float>(tick) / 4.0f;
		const float secondsAgo = durationSeconds * (1.0f - static_cast<float>(tick) / 4.0f);
		char label[24];
		std::snprintf(label, sizeof(label), "-%.2fs", secondsAgo);
		drawList->AddText(ImVec2(x - 15.0f, graphTop + graphHeight + 3.0f), color(180, 184, 192), label);
	}
	drawList->AddText(ImVec2(graphLeft + 4.0f, topLeft.y + 2.0f), color(86, 190, 255), "L");
	if (channels > 1)
		drawList->AddText(ImVec2(graphLeft + 18.0f, topLeft.y + 2.0f), color(255, 177, 75), "R");
}

void VisualizerPanel::calculateSpectrum()
{
	const std::size_t channelCount = mixer.getFormat().channels;
	const std::size_t availableFrames = channelCount == 0 ? 0 : audioSampleCount / channelCount;
	if (availableFrames < FftSize)
	{
		spectrumDb.fill(-72.0f);
		return;
	}

	const std::size_t firstFrame = availableFrames - FftSize;
	float windowSum = 0.0f;
	for (std::size_t i = 0; i < FftSize; ++i)
	{
		float monoSample = 0.0f;
		for (std::size_t channel = 0; channel < channelCount; ++channel)
			monoSample += audioSamples[(firstFrame + i) * channelCount + channel];
		monoSample /= static_cast<float>(channelCount);
		const float window = 0.5f - 0.5f * std::cos(2.0f * Pi * static_cast<float>(i) / static_cast<float>(FftSize - 1));
		windowSum += window;
		fftData[i] = std::complex<float>(monoSample * window, 0.0f);
	}

	for (std::size_t i = 1, j = 0; i < FftSize; ++i)
	{
		std::size_t bit = FftSize >> 1;
		for (; j & bit; bit >>= 1)
			j ^= bit;
		j ^= bit;
		if (i < j)
			std::swap(fftData[i], fftData[j]);
	}

	for (std::size_t length = 2; length <= FftSize; length <<= 1)
	{
		const float angle = -2.0f * Pi / static_cast<float>(length);
		const std::complex<float> step(std::cos(angle), std::sin(angle));
		for (std::size_t start = 0; start < FftSize; start += length)
		{
			std::complex<float> twiddle(1.0f, 0.0f);
			for (std::size_t offset = 0; offset < length / 2; ++offset)
			{
				const std::complex<float> even = fftData[start + offset];
				const std::complex<float> odd = fftData[start + offset + length / 2] * twiddle;
				fftData[start + offset] = even + odd;
				fftData[start + offset + length / 2] = even - odd;
				twiddle *= step;
			}
		}
	}

	const float sampleRate = static_cast<float>(mixer.getFormat().sampleRate);
	const float minFrequency = std::min(MinFrequencyHz, sampleRate * 0.01f);
	const float maxFrequency = std::max(minFrequency * 1.01f, sampleRate * 0.49f);
	for (std::size_t band = 0; band < SpectrumBinCount; ++band)
	{
		const float fraction0 = static_cast<float>(band) / static_cast<float>(SpectrumBinCount);
		const float fraction1 = static_cast<float>(band + 1) / static_cast<float>(SpectrumBinCount);
		const float frequency0 = minFrequency * std::pow(maxFrequency / minFrequency, fraction0);
		const float frequency1 = minFrequency * std::pow(maxFrequency / minFrequency, fraction1);
		const std::size_t firstBin = std::clamp<std::size_t>(
			static_cast<std::size_t>(frequency0 * static_cast<float>(FftSize) / sampleRate), 1, FftSize / 2);
		const std::size_t lastBin = std::clamp<std::size_t>(
			static_cast<std::size_t>(std::ceil(frequency1 * static_cast<float>(FftSize) / sampleRate)), firstBin + 1, FftSize / 2 + 1);
		float peakMagnitude = 0.0f;
		for (std::size_t bin = firstBin; bin < lastBin; ++bin)
			peakMagnitude = std::max(peakMagnitude, std::abs(fftData[bin]) * (2.0f / windowSum));
		spectrumDb[band] = std::max(-72.0f, 20.0f * std::log10(std::max(peakMagnitude, 1.0e-6f)));
	}
}

void VisualizerPanel::drawSpectrum()
{
	ImGui::TextUnformatted("Frequency spectrum (FFT)");
	const ImVec2 size(std::max(100.0f, ImGui::GetContentRegionAvail().x), 158.0f);
	const ImVec2 topLeft = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##SpectrumCanvas", size);
	const ImVec2 bottomRight(topLeft.x + size.x, topLeft.y + size.y);
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(topLeft, bottomRight, color(18, 21, 27));
	const float graphLeft = topLeft.x + 38.0f;
	const float graphTop = topLeft.y + 6.0f;
	const float graphWidth = size.x - 46.0f;
	const float graphHeight = size.y - 29.0f;
	for (int db = -60; db <= 0; db += 20)
	{
		const float y = graphTop + graphHeight * (1.0f - static_cast<float>(db + 72) / 72.0f);
		drawList->AddLine(ImVec2(graphLeft, y), ImVec2(graphLeft + graphWidth, y), db == 0 ? color(95, 102, 112) : color(52, 57, 65));
		const std::string label = std::to_string(db);
		drawList->AddText(ImVec2(topLeft.x + 2.0f, y - 7.0f), color(180, 184, 192), label.c_str());
	}
	const float sampleRate = static_cast<float>(mixer.getFormat().sampleRate);
	const float minFrequency = std::min(MinFrequencyHz, sampleRate * 0.01f);
	const float maxFrequency = std::max(minFrequency * 1.01f, sampleRate * 0.49f);
	const float frequencyTicks[] = { 20.0f, 50.0f, 100.0f, 200.0f, 500.0f, 1000.0f, 2000.0f, 5000.0f, 10000.0f, 20000.0f };
	for (float frequency : frequencyTicks)
	{
		if (frequency < minFrequency || frequency > maxFrequency)
			continue;
		const float x = graphLeft + graphWidth * std::log(frequency / minFrequency) / std::log(maxFrequency / minFrequency);
		drawList->AddLine(ImVec2(x, graphTop), ImVec2(x, graphTop + graphHeight), color(42, 47, 55));
		const std::string label = formatFrequency(frequency);
		drawList->AddText(ImVec2(x - 8.0f, graphTop + graphHeight + 3.0f), color(180, 184, 192), label.c_str());
	}
	for (std::size_t i = 0; i < SpectrumBinCount; ++i)
	{
		const float x0 = graphLeft + graphWidth * static_cast<float>(i) / static_cast<float>(SpectrumBinCount);
		const float x1 = graphLeft + graphWidth * static_cast<float>(i + 1) / static_cast<float>(SpectrumBinCount);
		const float normalized = std::clamp((spectrumDb[i] + 72.0f) / 72.0f, 0.0f, 1.0f);
		const float y = graphTop + graphHeight * (1.0f - normalized);
		drawList->AddRectFilled(ImVec2(x0 + 1.0f, y), ImVec2(x1 - 1.0f, graphTop + graphHeight), color(112, 210, 145));
	}
	if (audioSampleCount == 0)
		drawList->AddText(ImVec2(graphLeft + 8.0f, graphTop + 8.0f), color(180, 184, 192), "Waiting for audio output");
}

float VisualizerPanel::frequencyToX(float frequency, float minFrequency, float maxFrequency,
	float left, float width) const
{
	const float position = std::log(std::clamp(frequency, minFrequency, maxFrequency) / minFrequency)
		/ std::log(maxFrequency / minFrequency);
	return left + position * width;
}

float VisualizerPanel::xToFrequency(float x, float minFrequency, float maxFrequency,
	float left, float width) const
{
	const float position = std::clamp((x - left) / width, 0.0f, 1.0f);
	return minFrequency * std::pow(maxFrequency / minFrequency, position);
}

float VisualizerPanel::gainToY(float gainDb, float top, float height) const
{
	const float position = (MaxGainDb - gainDb) / (MaxGainDb - MinGainDb);
	return top + position * height;
}

float VisualizerPanel::yToGain(float y, float top, float height) const
{
	const float position = std::clamp((y - top) / height, 0.0f, 1.0f);
	return MaxGainDb - position * (MaxGainDb - MinGainDb);
}

void VisualizerPanel::drawEqualizerResponse()
{
	auto equalizer = effectsPanel.getEqualizer();
	std::size_t bandCount = equalizer->getBandCount();
	if (ImGui::Button("Add band") && equalizer->addBand({ 1000.0f, 0.0f, 1.0f }))
	{
		selectedEqualizerBand = static_cast<int>(equalizer->getBandCount() - 1);
		bandCount = equalizer->getBandCount();
	}
	ImGui::SameLine();
	if (bandCount > 1 && selectedEqualizerBand >= 0 && static_cast<std::size_t>(selectedEqualizerBand) < bandCount)
	{
		if (ImGui::Button("Remove selected band"))
		{
			equalizer->removeBand(static_cast<std::size_t>(selectedEqualizerBand));
			selectedEqualizerBand = std::max(0, selectedEqualizerBand - 1);
			bandCount = equalizer->getBandCount();
		}
	}
	ImGui::SameLine();
	ImGui::Text("Bands: %zu / %zu", bandCount, ParametricEqualizer::MaxBands);
	if (bandCount > 0)
	{
		selectedEqualizerBand = std::clamp(selectedEqualizerBand, 0, static_cast<int>(bandCount - 1));
		auto settings = equalizer->getBand(static_cast<std::size_t>(selectedEqualizerBand));
		if (ImGui::SliderFloat("Selected band Q", &settings.q, 0.1f, 10.0f, "%.2f"))
			equalizer->setBand(static_cast<std::size_t>(selectedEqualizerBand), settings);
		ImGui::SameLine();
		ImGui::Text("Band %d: %.0f Hz, %+.1f dB", selectedEqualizerBand + 1, settings.frequencyHz, settings.gainDb);
	}
	ImGui::TextUnformatted("EQ response (yellow)  |  Output spectrum after DSP (blue)");
	const ImVec2 size(std::max(100.0f, ImGui::GetContentRegionAvail().x), 240.0f);
	const ImVec2 topLeft = ImGui::GetCursorScreenPos();
	ImGui::InvisibleButton("##EQResponseCanvas", size);
	const ImVec2 bottomRight(topLeft.x + size.x, topLeft.y + size.y);
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	drawList->AddRectFilled(topLeft, bottomRight, color(18, 21, 27));

	const float maxFrequency = std::max(21.0f,
		std::min(20000.0f, static_cast<float>(mixer.getFormat().sampleRate) * 0.49f));
	const float minFrequency = std::min(MinFrequencyHz, maxFrequency * 0.5f);
	const float graphTop = topLeft.y + 8.0f;
	const float graphHeight = size.y - 29.0f;
	const float graphLeft = topLeft.x + 42.0f;
	const float graphWidth = size.x - 82.0f;
	const float graphRight = graphLeft + graphWidth;

	const ImVec2 mouse = ImGui::GetIO().MousePos;
	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	{
		float closestDistance = 13.0f;
		draggedEqualizerBand = -1;
		for (std::size_t band = 0; band < bandCount; ++band)
		{
			const auto settings = equalizer->getBand(band);
			const float x = frequencyToX(settings.frequencyHz, minFrequency, maxFrequency, graphLeft, graphWidth);
			const float y = gainToY(settings.gainDb, graphTop, graphHeight);
			const float dx = mouse.x - x;
			const float dy = mouse.y - y;
			const float distance = std::sqrt(dx * dx + dy * dy);
			if (distance < closestDistance)
			{
				closestDistance = distance;
				draggedEqualizerBand = static_cast<int>(band);
			}
		}
		if (draggedEqualizerBand >= 0)
			selectedEqualizerBand = draggedEqualizerBand;
		else if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && bandCount < ParametricEqualizer::MaxBands)
		{
			const float frequency = xToFrequency(mouse.x, minFrequency, maxFrequency, graphLeft, graphWidth);
			const float gain = yToGain(mouse.y, graphTop, graphHeight);
			if (equalizer->addBand({ frequency, gain, 1.0f }))
			{
				selectedEqualizerBand = static_cast<int>(equalizer->getBandCount() - 1);
				bandCount = equalizer->getBandCount();
				draggedEqualizerBand = selectedEqualizerBand;
			}
		}
	}

	if (draggedEqualizerBand >= 0 && ImGui::IsMouseDown(ImGuiMouseButton_Left))
	{
		const auto settings = EqualizerBandSettings{
			xToFrequency(mouse.x, minFrequency, maxFrequency, graphLeft, graphWidth),
			yToGain(mouse.y, graphTop, graphHeight),
			equalizer->getBand(static_cast<std::size_t>(draggedEqualizerBand)).q
		};
		equalizer->setBand(static_cast<std::size_t>(draggedEqualizerBand), settings);
	}
	if (!ImGui::IsMouseDown(ImGuiMouseButton_Left))
		draggedEqualizerBand = -1;

	for (int db = -24; db <= 24; db += 12)
	{
		const float y = gainToY(static_cast<float>(db), graphTop, graphHeight);
		drawList->AddLine(ImVec2(graphLeft, y), ImVec2(graphRight, y), color(db == 0 ? 100 : 52, db == 0 ? 106 : 57, db == 0 ? 115 : 65));
		const std::string label = (db > 0 ? "+" : "") + std::to_string(db);
		drawList->AddText(ImVec2(topLeft.x + 2.0f, y - 7.0f), color(230, 190, 90), label.c_str());
	}
	for (int dbfs = -72; dbfs <= 0; dbfs += 24)
	{
		const float y = graphTop + graphHeight * (1.0f - static_cast<float>(dbfs + 72) / 72.0f);
		const std::string label = std::to_string(dbfs);
		drawList->AddText(ImVec2(graphRight + 4.0f, y - 7.0f), color(86, 190, 255), label.c_str());
	}
	const float tickFrequencies[] = { 20.0f, 100.0f, 1000.0f, 10000.0f, 20000.0f };
	for (float frequency : tickFrequencies)
	{
		if (frequency < minFrequency || frequency > maxFrequency)
			continue;
		const float x = frequencyToX(frequency, minFrequency, maxFrequency, graphLeft, graphWidth);
		drawList->AddLine(ImVec2(x, graphTop), ImVec2(x, graphTop + graphHeight), color(52, 57, 65));
		const std::string label = formatFrequency(frequency);
		drawList->AddText(ImVec2(x - 8.0f, graphTop + graphHeight + 3.0f), color(180, 184, 192), label.c_str());
	}

	const std::size_t responsePoints = std::max<std::size_t>(2, static_cast<std::size_t>(graphWidth));
	ImVec2 previous{};
	for (std::size_t i = 0; i < responsePoints; ++i)
	{
		const float fraction = static_cast<float>(i) / static_cast<float>(responsePoints - 1);
		const float frequency = minFrequency * std::pow(maxFrequency / minFrequency, fraction);
		const float responseDb = std::clamp(equalizer->getFrequencyResponseDb(frequency, mixer.getFormat().sampleRate), MinGainDb, MaxGainDb);
		const ImVec2 point(graphLeft + fraction * graphWidth, gainToY(responseDb, graphTop, graphHeight));
		if (i > 0)
			drawList->AddLine(previous, point, color(255, 190, 75), 2.0f);
		previous = point;
	}

	const float sampleRate = static_cast<float>(mixer.getFormat().sampleRate);
	const float spectrumMinFrequency = std::min(MinFrequencyHz, sampleRate * 0.01f);
	const float spectrumMaxFrequency = std::max(spectrumMinFrequency * 1.01f, sampleRate * 0.49f);
	for (std::size_t band = 0; band < SpectrumBinCount; ++band)
	{
		const float fraction = (static_cast<float>(band) + 0.5f) / static_cast<float>(SpectrumBinCount);
		const float frequency = spectrumMinFrequency * std::pow(spectrumMaxFrequency / spectrumMinFrequency, fraction);
		if (frequency < minFrequency || frequency > maxFrequency)
			continue;
		const float x = frequencyToX(frequency, minFrequency, maxFrequency, graphLeft, graphWidth);
		const float normalized = std::clamp((spectrumDb[band] + 72.0f) / 72.0f, 0.0f, 1.0f);
		const ImVec2 point(x, graphTop + (1.0f - normalized) * graphHeight);
		if (band > 0)
		{
			const float previousFraction = (static_cast<float>(band - 1) + 0.5f) / static_cast<float>(SpectrumBinCount);
			const float previousFrequency = spectrumMinFrequency * std::pow(spectrumMaxFrequency / spectrumMinFrequency, previousFraction);
			const float previousNormalized = std::clamp((spectrumDb[band - 1] + 72.0f) / 72.0f, 0.0f, 1.0f);
			const ImVec2 previousSpectrum(frequencyToX(previousFrequency, minFrequency, maxFrequency, graphLeft, graphWidth),
				graphTop + (1.0f - previousNormalized) * graphHeight);
			drawList->AddLine(previousSpectrum, point, color(86, 190, 255), 2.0f);
		}
	}
	for (std::size_t band = 0; band < bandCount; ++band)
	{
		const auto settings = equalizer->getBand(band);
		const ImVec2 point(frequencyToX(settings.frequencyHz, minFrequency, maxFrequency, graphLeft, graphWidth),
			gainToY(settings.gainDb, graphTop, graphHeight));
		const bool selected = static_cast<int>(band) == selectedEqualizerBand;
		drawList->AddCircleFilled(point, selected ? 7.0f : 5.5f, color(255, 190, 75));
		drawList->AddCircle(point, selected ? 9.0f : 7.0f, selected ? color(255, 235, 170) : color(140, 100, 45), 0, 1.5f);
	}
	drawList->AddText(ImVec2(topLeft.x + 2.0f, graphTop + 2.0f), color(230, 190, 90), "EQ dB");
	drawList->AddText(ImVec2(graphRight + 3.0f, graphTop + 2.0f), color(86, 190, 255), "dBFS");
	if (!effectsPanel.isEqualizerEnabled())
		drawList->AddText(ImVec2(graphLeft + 8.0f, graphTop + 8.0f), color(210, 185, 120), "EQ curve preview (bypassed)");
}

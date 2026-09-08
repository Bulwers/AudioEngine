#pragma once

#include <vector>
#include <string>
#include <cstdint>

class AudioBuffer
{
public:

	AudioBuffer() = default;
	AudioBuffer(std::string filePath, uint16_t channels, uint32_t sampleRate, uint16_t bitsPerSample, const std::vector<float>& samples);

	const std::vector<float>& getSamples() const { return samples; }
	std::vector<float>& getSamples() { return samples; }

	uint16_t getChannels() const { return channels; }
	uint32_t getSampleRate() const { return sampleRate; }
	std::string getFilePath() const { return filePath; }


	// frame and channel are zero-based indexes
	// For example, for a stereo buffer(2 channels)
	// frame 0 channel 0 is the first sample of the left channel, and frame 0 channel 1 is the first sample of the right channel
	float getSample(std::size_t frame, std::size_t channel) const;
	void setSample(std::size_t frame, std::size_t channel, float value);

	std::size_t getFrameCount() const { if (channels == 0) return 0; return samples.size() / channels; }
	std::size_t getSampleCount() const { return samples.size(); }

	double getDuration() const { if (sampleRate == 0) return 0.0; return static_cast<double>(getFrameCount()) / static_cast<double>(sampleRate); }

private:

	std::string filePath;
	uint16_t channels = 0;
	uint32_t sampleRate = 0;
	uint16_t bitsPerSample = 0;
	std::vector<float> samples;

};
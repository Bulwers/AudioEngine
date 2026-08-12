#include <gtest/gtest.h>

#include "ResourceManager.h"
#include "AudioBuffer.h"
#include "AudioClip.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <cstdint>


namespace
{ 
	void writeUint16(std::ofstream& file, uint16_t value)
	{
		file.put(static_cast<char>(value & 0xFF));
		file.put(static_cast<char>((value >> 8) & 0xFF));
	}

	void writeUint32(std::ofstream& file, uint32_t value)
	{
		file.put(static_cast<char>(value & 0xFF));
		file.put(static_cast<char>((value >> 8) & 0xFF));
		file.put(static_cast<char>((value >> 16) & 0xFF));
		file.put(static_cast<char>((value >> 24) & 0xFF));
	}

	void writeTestWavFile(
		const std::filesystem::path& filePath,
		uint16_t channels,
		uint32_t sampleRate,
		const std::vector<int16_t>& samples)
	{
		std::ofstream file(filePath, std::ios::binary);
		if (!file)
		{
			throw std::runtime_error("Failed to create test WAV file.");
		}

		const uint16_t bitsPerSample = 16;
		const uint16_t blockAlign = static_cast<uint16_t>(channels * (bitsPerSample / 8));
		const uint32_t byteRate = sampleRate * blockAlign;
		const uint32_t dataSize = static_cast<uint32_t>(samples.size() * sizeof(int16_t));
		const uint32_t riffChunkSize = 36u + dataSize;

		file.write("RIFF", 4);
		writeUint32(file, riffChunkSize);
		file.write("WAVE", 4);

		file.write("fmt ", 4);
		writeUint32(file, 16);
		writeUint16(file, 1);
		writeUint16(file, channels);
		writeUint32(file, sampleRate);
		writeUint32(file, byteRate);
		writeUint16(file, blockAlign);
		writeUint16(file, bitsPerSample);

		file.write("data", 4);
		writeUint32(file, dataSize);

		for (int16_t sample : samples)
		{
			writeUint16(file, static_cast<uint16_t>(sample));
		}
	}
}

TEST(ResourceManagerTests, LoadAudioBuffersAndClips)
{
	ResourceManager manager;

	const std::filesystem::path wavPath = "Assets/SoundSrc/laser.wav";
	const std::filesystem::path savedClipPath = "Assets/AudioClips/laser_saved.dat";

	writeTestWavFile(wavPath, 1, 44100, { 0, 32767, -32768, 16384, -16384 });

	manager.loadAudioBuffers();
	manager.getAudioBuffers();
	std::cout << "audioBuffers" << "\n";

	manager.loadAudioClips();
	manager.getAudioClips();
	std::cout << "audioClips1" << "\n";

	manager.createAudioClips();
	manager.getAudioClips();
	std::cout << "audioClips2" << "\n";

	manager.saveAudioClips();
	std::cout << "ClipsSaved" << "\n";
}
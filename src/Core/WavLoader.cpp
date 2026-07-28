#include "WavLoader.h"

#include <fstream>
#include <iostream>
#include <cstring>
#include <iomanip>
#include <bit>

namespace
{
#pragma pack(push, 1)
	struct RiffHeader
	{
		char chunkId[4]; // "RIFF"
		uint32_t chunkSize;
		char format[4]; // "WAVE"
	};

	struct ChunkHeader
	{
		char chunkId[4];
		uint32_t chunkSize;
	};

	struct FmtChunk
	{
		uint16_t audioFormat;
		uint16_t numChannels;
		uint32_t sampleRate;
		uint32_t byteRate;
		uint16_t blockAlign;
		uint16_t bitsPerSample;
	};
#pragma pack(pop)

	constexpr uint16_t WaveFormatPcm = 0x0001;
	constexpr uint16_t WaveFormatIeeeFloat = 0x0003;
	constexpr uint16_t WaveFormatExtensible = 0xFFFE;

	bool chunkEquals(const char* chunkId, const char* expected)
	{
		return std::memcmp(chunkId, expected, 4) == 0;
	}

	uint16_t getExtensibleSubFormat(const std::vector<uint8_t>& fmtExtensionData)
	{
		if (fmtExtensionData.size() < 24)
		{
			throw std::runtime_error("Invalid WAVE_FORMAT_EXTENSIBLE fmt chunk.");
		}

		static constexpr uint8_t expectedGuidTail[14] =
		{
			0x00, 0x00, 0x00, 0x00,
			0x10, 0x00,
			0x80, 0x00,
			0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
		};

		const size_t subFormatOffset = 8;
		if (std::memcmp(fmtExtensionData.data() + subFormatOffset + 2, expectedGuidTail, sizeof(expectedGuidTail)) != 0)
		{
			throw std::runtime_error("Unsupported WAVE_FORMAT_EXTENSIBLE sub-format GUID.");
		}

		return static_cast<uint16_t>(
			static_cast<uint16_t>(fmtExtensionData[subFormatOffset]) |
			(static_cast<uint16_t>(fmtExtensionData[subFormatOffset + 1]) << 8));
	}
}

std::shared_ptr<AudioBuffer> WavLoader::loadFile(const std::string& filePath) const
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file)
	{
		throw std::runtime_error("Failed to open WAV file: " + filePath);
	}

	RiffHeader riff{};
	file.read(reinterpret_cast<char*>(&riff), sizeof(riff));
	if (!chunkEquals(riff.chunkId, "RIFF") || !chunkEquals(riff.format, "WAVE"))
	{
		throw std::runtime_error("Invalid WAV file: " + filePath);
	}

	FmtChunk fmtData{};
	std::vector<uint8_t> fmtExtensionData;
	bool hasFmtChunk = false;
	std::vector<uint8_t> rawAudioData;

	while (file)
	{
		ChunkHeader chunk{};
		if (!file.read(reinterpret_cast<char*>(&chunk), sizeof(chunk)))
			break;

		if (chunkEquals(chunk.chunkId, "fmt "))
		{
			if (chunk.chunkSize < sizeof(FmtChunk))
			{
				throw std::runtime_error("Invalid fmt chunk size in WAV file: " + filePath);
			}

			file.read(reinterpret_cast<char*>(&fmtData), sizeof(fmtData));
			if (!file)
			{
				throw std::runtime_error("Failed to read fmt chunk in WAV file: " + filePath);
			}

			const uint32_t remainingFmt = chunk.chunkSize - static_cast<uint32_t>(sizeof(FmtChunk));
			fmtExtensionData.clear();

			if (remainingFmt > 0)
			{
				fmtExtensionData.resize(remainingFmt);
				file.read(reinterpret_cast<char*>(fmtExtensionData.data()), remainingFmt);
				if (!file)
				{
					throw std::runtime_error("Failed to read fmt extension in WAV file: " + filePath);
				}
			}

			hasFmtChunk = true;
		}
		else if (chunkEquals(chunk.chunkId, "data"))
		{
			rawAudioData.resize(chunk.chunkSize);
			file.read(reinterpret_cast<char*>(rawAudioData.data()), chunk.chunkSize);
			if (!file)
			{
				throw std::runtime_error("Failed to read data chunk in WAV file: " + filePath);
			}
		}
		else
		{
			file.seekg(chunk.chunkSize, std::ios::cur);
		}

		if (chunk.chunkSize % 2 != 0)
		{
			file.seekg(1, std::ios::cur);
		}
	}

	if (!hasFmtChunk)
	{
		throw std::runtime_error("Missing fmt chunk in WAV file: " + filePath);
	}
	if (rawAudioData.empty())
	{
		throw std::runtime_error("Missing data chunk in WAV file: " + filePath);
	}
	
	uint16_t finalAudioFormat = fmtData.audioFormat;
	if (finalAudioFormat == WaveFormatExtensible)
	{
		finalAudioFormat = getExtensibleSubFormat(fmtExtensionData);
	}

	validateFormat(finalAudioFormat, fmtData.bitsPerSample);

	std::vector<float> bufferSamples = convertSamplesToFloat(finalAudioFormat, fmtData.bitsPerSample, rawAudioData);

	return std::make_shared<AudioBuffer>(fmtData.numChannels, fmtData.sampleRate, fmtData.bitsPerSample, bufferSamples);
}

void WavLoader::validateFormat(uint16_t audioFormat, uint16_t bitsPerSample)
{
	switch (audioFormat)
	{
	case WaveFormatPcm:
		switch (bitsPerSample)
		{
		case 8:
		case 16:				
		case 24:
		case 32:
			return;

		default:
			throw std::runtime_error("Unsupported bits per sample in WAV file (PCM): " + std::to_string(bitsPerSample));
		}

	case WaveFormatIeeeFloat:
		switch (bitsPerSample)
		{
		case 32:
		case 64:
			return;

		default:
			throw std::runtime_error("Unsupported bits per sample in WAV file (IEEE): " + std::to_string(bitsPerSample));
		}

	default:
		throw std::runtime_error("Unsupported audio format in WAV file: " + std::to_string(audioFormat));
	}
}

std::vector<float> WavLoader::convertSamplesToFloat(uint16_t audioFormat, uint16_t bitsPerSample, const std::vector<uint8_t>& rawAudioData)
{
	if (audioFormat == WaveFormatPcm)
	{
		switch (bitsPerSample)
		{
		case 8:
			return decodePcm8Samples(rawAudioData);

		case 16:
			return decodePcm16Samples(rawAudioData);

		case 24:
			return decodePcm24Samples(rawAudioData);

		case 32:
			return decodePcm32Samples(rawAudioData);

		default:
			throw std::runtime_error("Unsupported bit depth: " + std::to_string(bitsPerSample));
		}
	}
	else if (audioFormat == WaveFormatIeeeFloat)
	{
		switch (bitsPerSample)
		{
		case 32:
			return decodeIeeeFloat32Samples(rawAudioData);

		case 64:
			return decodeIeeeFloat64Samples(rawAudioData);

		default:
			throw std::runtime_error("Unsupported bit depth: " + std::to_string(bitsPerSample));
		}
	}
	else
	{
		throw std::runtime_error("Unsupported audio format: " + std::to_string(audioFormat));
	}
}

// Helper functions for decoding PCM samples to float

std::vector<float> WavLoader::decodePcm8Samples(const std::vector<uint8_t>& rawAudioData)
{
	std::vector<float> samples;
	samples.reserve(rawAudioData.size());

	for (uint8_t sample : rawAudioData)
	{
		samples.push_back(convertPcm8ToFloat(sample));
	}

	return samples;
}

std::vector<float> WavLoader::decodePcm16Samples(const std::vector<uint8_t>& rawAudioData)
{
	std::vector<float> samples;
	samples.reserve(rawAudioData.size() / 2);

	for (size_t offset = 0; offset < rawAudioData.size(); offset += 2)
	{
		int16_t sample = readLittleEndianInt16(rawAudioData, offset);

		samples.push_back(convertPcm16ToFloat(sample));
	}
	return samples;
}

std::vector<float> WavLoader::decodePcm24Samples(const std::vector<uint8_t>& rawAudioData)
{
	if ((rawAudioData.size() % 3) != 0)
	{
		throw std::runtime_error("Invalid 24-bit PCM data size.");
	}

	std::vector<float> samples;
	samples.reserve(rawAudioData.size() / 3);

	for (size_t offset = 0; offset < rawAudioData.size(); offset += 3)
	{
		int32_t sample = readLittleEndianInt24(rawAudioData, offset);

		samples.push_back(convertPcm24ToFloat(sample));
	}
	return samples;
}

std::vector<float> WavLoader::decodePcm32Samples(const std::vector<uint8_t>& rawAudioData)
{
	if ((rawAudioData.size() % 4) != 0)
	{
		throw std::runtime_error("Invalid 32-bit PCM data size.");
	}

	std::vector<float> samples;
	samples.reserve(rawAudioData.size() / 4);

	for (size_t offset = 0; offset < rawAudioData.size(); offset += 4)
	{
		int32_t sample = readLittleEndianInt32(rawAudioData, offset);

		samples.push_back(convertPcm32ToFloat(sample));
	}
	return samples;
}

// Helper functions for decoding IEEE float samples to float

std::vector<float> WavLoader::decodeIeeeFloat32Samples(const std::vector<uint8_t>& rawAudioData)
{
	if ((rawAudioData.size() % 4) != 0)
	{
		throw std::runtime_error("Invalid 32-bit IEEE float data size.");
	}

	std::vector<float> samples;
	samples.reserve(rawAudioData.size() / 4);

	for (size_t offset = 0; offset < rawAudioData.size(); offset += 4)
	{
		samples.push_back(readLittleEndianFloat32(rawAudioData, offset));
	}

	return samples;
}

std::vector<float> WavLoader::decodeIeeeFloat64Samples(const std::vector<uint8_t>& rawAudioData)
{
	if ((rawAudioData.size() % 8) != 0)
	{
		throw std::runtime_error("Invalid 64-bit IEEE float data size.");
	}

	std::vector<float> samples;
	samples.reserve(rawAudioData.size() / 8);

	for (size_t offset = 0; offset < rawAudioData.size(); offset += 8)
	{
		samples.push_back(static_cast<float>(readLittleEndianFloat64(rawAudioData, offset)));
	}

	return samples;
}

// Helper functions for reading little-endian values from byte arrays

int32_t WavLoader::readLittleEndianInt16(const std::vector<uint8_t>& bytes, size_t offset)
{
	uint16_t value =
		static_cast<uint16_t>(bytes[offset]) |
		(static_cast<uint16_t>(bytes[offset + 1]) << 8);

	return static_cast<int16_t>(value);
}

int32_t WavLoader::readLittleEndianInt24(const std::vector<uint8_t>& bytes, size_t offset)
{
	int32_t value =
		static_cast<int32_t>(bytes[offset]) |
		(static_cast<int32_t>(bytes[offset + 1]) << 8) |
		(static_cast<int32_t>(bytes[offset + 2]) << 16);

	if ((value & 0x00800000) != 0)
	{
		value |= ~0x00FFFFFF;
	}
	return value;
}

int32_t WavLoader::readLittleEndianInt32(const std::vector<uint8_t>& bytes, size_t offset)
{
	uint32_t value =
		static_cast<uint32_t>(bytes[offset]) |
		(static_cast<uint32_t>(bytes[offset + 1]) << 8) |
		(static_cast<uint32_t>(bytes[offset + 2]) << 16) |
		(static_cast<uint32_t>(bytes[offset + 3]) << 24);

	return static_cast<int32_t>(value);
}

// Helper functions for reading little-endian float values from byte arrays

float WavLoader::readLittleEndianFloat32(const std::vector<uint8_t>& bytes, size_t offset)
{
	uint32_t value =
		static_cast<uint32_t>(bytes[offset]) |
		(static_cast<uint32_t>(bytes[offset + 1]) << 8) |
		(static_cast<uint32_t>(bytes[offset + 2]) << 16) |
		(static_cast<uint32_t>(bytes[offset + 3]) << 24);

	return std::bit_cast<float>(value);
}

double WavLoader::readLittleEndianFloat64(const std::vector<uint8_t>& bytes, size_t offset)
{
	uint64_t value =
		static_cast<uint64_t>(bytes[offset]) |
		(static_cast<uint64_t>(bytes[offset + 1]) << 8) |
		(static_cast<uint64_t>(bytes[offset + 2]) << 16) |
		(static_cast<uint64_t>(bytes[offset + 3]) << 24) |
		(static_cast<uint64_t>(bytes[offset + 4]) << 32) |
		(static_cast<uint64_t>(bytes[offset + 5]) << 40) |
		(static_cast<uint64_t>(bytes[offset + 6]) << 48) |
		(static_cast<uint64_t>(bytes[offset + 7]) << 56);

	return std::bit_cast<double>(value);
}

// Conversion functions for PCM formats to float

float WavLoader::convertPcm8ToFloat(uint8_t value)
{
	return (static_cast<int>(value) - 128) / 128.0f;
}

float WavLoader::convertPcm16ToFloat(int16_t value)
{
	return (static_cast<float>(value) / 32768.0f);
}

float WavLoader::convertPcm24ToFloat(int32_t value)
{
	return (static_cast<float>(value) / 8388608.0f);
}

float WavLoader::convertPcm32ToFloat(int32_t value)
{
	return (static_cast<float>(value) / 2147483648.0f);
}
#pragma once

#include "AudioBuffer.h"

#include <vector>
#include <cstdint>
#include <string>
#include <memory>

class WavLoader
{
public:

	std::shared_ptr<AudioBuffer> loadFile(const std::string& filePath) const;

private:

	std::vector<float> convertSamplesToFloat(uint16_t audioFormat, uint16_t bitsPerSample, std::vector<uint8_t>& rawAudioData);

	static void validateFormat(uint16_t audioFormat, uint16_t bitsPerSample);

	static std::vector<float> decodePcm8Samples(const std::vector<uint8_t>& rawAudioData);
	static std::vector<float> decodePcm16Samples(const std::vector<uint8_t>& rawAudioData);
	static std::vector<float> decodePcm24Samples(const std::vector<uint8_t>& rawAudioData);
	static std::vector<float> decodePcm32Samples(const std::vector<uint8_t>& rawAudioData);

	static std::vector<float> decodeIeeeFloat32Samples(const std::vector<uint8_t>& rawAudioData);
	static std::vector<float> decodeIeeeFloat64Samples(const std::vector<uint8_t>& rawAudioData);

	static int32_t readLittleEndianInt16(const std::vector<uint8_t>& bytes, size_t offset);
	static int32_t readLittleEndianInt24(const std::vector<uint8_t>& bytes, size_t offset);
	static int32_t readLittleEndianInt32(const std::vector<uint8_t>& bytes, size_t offset);

	static float readLittleEndianFloat32(const std::vector<uint8_t>& bytes, size_t offset);
	static double readLittleEndianFloat64(const std::vector<uint8_t>& bytes, size_t offset);

	static float convertPcm8ToFloat(uint8_t value);
	static float convertPcm16ToFloat(int16_t value);
	static float convertPcm24ToFloat(int32_t value);
	static float convertPcm32ToFloat(int32_t value);

};

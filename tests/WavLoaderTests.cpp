#include <gtest/gtest.h>

#include "WavLoader.h"

#include <array>
#include <bit>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace
{
	struct Chunk
	{
		std::array<char, 4> id;
		std::vector<uint8_t> data;
	};

	std::filesystem::path getTestDirectory()
	{
		const auto path = std::filesystem::temp_directory_path() / "AudioEngineTests";
		std::filesystem::create_directories(path);
		return path;
	}

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

	uint32_t getPaddedChunkDataSize(uint32_t chunkSize)
	{
		return chunkSize + (chunkSize % 2u);
	}

	void writeChunk(std::ofstream& file, const Chunk& chunk)
	{
		file.write(chunk.id.data(), 4);
		writeUint32(file, static_cast<uint32_t>(chunk.data.size()));

		if (!chunk.data.empty())
		{
			file.write(reinterpret_cast<const char*>(chunk.data.data()), static_cast<std::streamsize>(chunk.data.size()));
		}

		if ((chunk.data.size() % 2u) != 0u)
		{
			file.put('\0');
		}
	}

	void writeRiffFile(
		const std::filesystem::path& filePath,
		const std::array<char, 4>& riffId,
		const std::array<char, 4>& formatId,
		const std::vector<Chunk>& chunks)
	{
		std::ofstream file(filePath, std::ios::binary);
		if (!file)
		{
			throw std::runtime_error("Failed to create test file.");
		}

		uint32_t riffChunkSize = 4;
		for (const Chunk& chunk : chunks)
		{
			riffChunkSize += 8u + getPaddedChunkDataSize(static_cast<uint32_t>(chunk.data.size()));
		}

		file.write(riffId.data(), 4);
		writeUint32(file, riffChunkSize);
		file.write(formatId.data(), 4);

		for (const Chunk& chunk : chunks)
		{
			writeChunk(file, chunk);
		}
	}

	std::vector<uint8_t> createFmtChunkData(
		uint16_t audioFormat,
		uint16_t channels,
		uint32_t sampleRate,
		uint16_t bitsPerSample,
		const std::vector<uint8_t>& extensionData = {})
	{
		std::vector<uint8_t> data;
		data.reserve(16 + extensionData.size());

		const uint16_t bytesPerSample = static_cast<uint16_t>(bitsPerSample / 8);
		const uint16_t blockAlign = static_cast<uint16_t>(channels * bytesPerSample);
		const uint32_t byteRate = sampleRate * blockAlign;

		data.push_back(static_cast<uint8_t>(audioFormat & 0xFF));
		data.push_back(static_cast<uint8_t>((audioFormat >> 8) & 0xFF));

		data.push_back(static_cast<uint8_t>(channels & 0xFF));
		data.push_back(static_cast<uint8_t>((channels >> 8) & 0xFF));

		data.push_back(static_cast<uint8_t>(sampleRate & 0xFF));
		data.push_back(static_cast<uint8_t>((sampleRate >> 8) & 0xFF));
		data.push_back(static_cast<uint8_t>((sampleRate >> 16) & 0xFF));
		data.push_back(static_cast<uint8_t>((sampleRate >> 24) & 0xFF));

		data.push_back(static_cast<uint8_t>(byteRate & 0xFF));
		data.push_back(static_cast<uint8_t>((byteRate >> 8) & 0xFF));
		data.push_back(static_cast<uint8_t>((byteRate >> 16) & 0xFF));
		data.push_back(static_cast<uint8_t>((byteRate >> 24) & 0xFF));

		data.push_back(static_cast<uint8_t>(blockAlign & 0xFF));
		data.push_back(static_cast<uint8_t>((blockAlign >> 8) & 0xFF));

		data.push_back(static_cast<uint8_t>(bitsPerSample & 0xFF));
		data.push_back(static_cast<uint8_t>((bitsPerSample >> 8) & 0xFF));

		data.insert(data.end(), extensionData.begin(), extensionData.end());

		return data;
	}

	std::vector<uint8_t> createPcm16Data(const std::vector<int16_t>& samples)
	{
		std::vector<uint8_t> data;
		data.reserve(samples.size() * sizeof(int16_t));

		for (int16_t sample : samples)
		{
			const uint16_t value = static_cast<uint16_t>(sample);
			data.push_back(static_cast<uint8_t>(value & 0xFF));
			data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
		}

		return data;
	}

	std::vector<uint8_t> createFloat32Data(const std::vector<float>& samples)
	{
		std::vector<uint8_t> data;
		data.reserve(samples.size() * sizeof(float));

		for (float sample : samples)
		{
			const uint32_t value = std::bit_cast<uint32_t>(sample);

			data.push_back(static_cast<uint8_t>(value & 0xFF));
			data.push_back(static_cast<uint8_t>((value >> 8) & 0xFF));
			data.push_back(static_cast<uint8_t>((value >> 16) & 0xFF));
			data.push_back(static_cast<uint8_t>((value >> 24) & 0xFF));
		}

		return data;
	}

	std::vector<uint8_t> createExtensibleFmtData(uint16_t subFormat, uint16_t validBitsPerSample)
	{
		return
		{
			0x16, 0x00,
			static_cast<uint8_t>(validBitsPerSample & 0xFF), static_cast<uint8_t>((validBitsPerSample >> 8) & 0xFF),
			0x00, 0x00, 0x00, 0x00,
			static_cast<uint8_t>(subFormat & 0xFF), static_cast<uint8_t>((subFormat >> 8) & 0xFF),
			0x00, 0x00, 0x00, 0x00,
			0x10, 0x00,
			0x80, 0x00,
			0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
		};
	}

	void writeWavFile(
		const std::filesystem::path& filePath,
		uint16_t audioFormat,
		uint16_t channels,
		uint32_t sampleRate,
		uint16_t bitsPerSample,
		const std::vector<uint8_t>& sampleData,
		const std::vector<Chunk>& extraChunks = {},
		const std::vector<uint8_t>& fmtExtensionData = {})
	{
		std::vector<Chunk> chunks;
		chunks.push_back({ { 'f', 'm', 't', ' ' }, createFmtChunkData(audioFormat, channels, sampleRate, bitsPerSample, fmtExtensionData) });

		for (const Chunk& chunk : extraChunks)
		{
			chunks.push_back(chunk);
		}

		chunks.push_back({ { 'd', 'a', 't', 'a' }, sampleData });

		writeRiffFile(filePath, { 'R', 'I', 'F', 'F' }, { 'W', 'A', 'V', 'E' }, chunks);
	}
}

TEST(WavLoaderTests, LoadsPcm16MonoSamplesAndMetadata)
{
	const auto filePath = getTestDirectory() / "wavloader_pcm16_mono.wav";
	writeWavFile(filePath, 0x0001, 1, 44100, 16, createPcm16Data({ 0, 32767, -32768 }));

	WavLoader loader;
	const auto buffer = loader.loadFile(filePath.string());

	ASSERT_NE(buffer, nullptr);
	EXPECT_EQ(buffer->getChannels(), 1);
	EXPECT_EQ(buffer->getSampleRate(), 44100u);
	EXPECT_EQ(buffer->getSampleCount(), 3u);
	EXPECT_EQ(buffer->getFilePath(), filePath.string());

	const auto& samples = buffer->getSamples();
	ASSERT_EQ(samples.size(), 3u);
	EXPECT_NEAR(samples[0], 0.0f, 1e-6f);
	EXPECT_NEAR(samples[1], 32767.0f / 32768.0f, 1e-6f);
	EXPECT_NEAR(samples[2], -1.0f, 1e-6f);
}

TEST(WavLoaderTests, LoadsIeeeFloat32StereoSamples)
{
	const auto filePath = getTestDirectory() / "wavloader_float32_stereo.wav";
	writeWavFile(filePath, 0x0003, 2, 48000, 32, createFloat32Data({ -1.0f, -0.5f, 0.25f, 1.0f }));

	WavLoader loader;
	const auto buffer = loader.loadFile(filePath.string());

	ASSERT_NE(buffer, nullptr);
	EXPECT_EQ(buffer->getChannels(), 2);
	EXPECT_EQ(buffer->getSampleRate(), 48000u);
	EXPECT_EQ(buffer->getFrameCount(), 2u);

	const auto& samples = buffer->getSamples();
	ASSERT_EQ(samples.size(), 4u);
	EXPECT_FLOAT_EQ(samples[0], -1.0f);
	EXPECT_FLOAT_EQ(samples[1], -0.5f);
	EXPECT_FLOAT_EQ(samples[2], 0.25f);
	EXPECT_FLOAT_EQ(samples[3], 1.0f);
}

TEST(WavLoaderTests, LoadsWaveFormatExtensiblePcmSamples)
{
	const auto filePath = getTestDirectory() / "wavloader_extensible_pcm.wav";
	writeWavFile(
		filePath,
		0xFFFE,
		1,
		22050,
		16,
		createPcm16Data({ 16384, -16384 }),
		{},
		createExtensibleFmtData(0x0001, 16));

	WavLoader loader;
	const auto buffer = loader.loadFile(filePath.string());

	ASSERT_NE(buffer, nullptr);
	ASSERT_EQ(buffer->getSamples().size(), 2u);
	EXPECT_EQ(buffer->getChannels(), 1);
	EXPECT_EQ(buffer->getSampleRate(), 22050u);
	EXPECT_NEAR(buffer->getSamples()[0], 0.5f, 1e-6f);
	EXPECT_NEAR(buffer->getSamples()[1], -0.5f, 1e-6f);
}

TEST(WavLoaderTests, SkipsUnknownChunksAndOddSizedPadding)
{
	const auto filePath = getTestDirectory() / "wavloader_junk_chunk.wav";

	const std::vector<Chunk> extraChunks =
	{
		{ { 'J', 'U', 'N', 'K' }, { 0x10, 0x20, 0x30 } }
	};

	writeWavFile(filePath, 0x0001, 1, 44100, 16, createPcm16Data({ 0, 16384 }), extraChunks);

	WavLoader loader;
	const auto buffer = loader.loadFile(filePath.string());

	ASSERT_NE(buffer, nullptr);
	ASSERT_EQ(buffer->getSamples().size(), 2u);
	EXPECT_NEAR(buffer->getSamples()[0], 0.0f, 1e-6f);
	EXPECT_NEAR(buffer->getSamples()[1], 0.5f, 1e-6f);
}

TEST(WavLoaderTests, ThrowsForInvalidRiffHeader)
{
	const auto filePath = getTestDirectory() / "wavloader_invalid_header.wav";

	writeRiffFile(
		filePath,
		{ 'R', 'I', 'F', 'X' },
		{ 'W', 'A', 'V', 'E' },
		{
			{ { 'f', 'm', 't', ' ' }, createFmtChunkData(0x0001, 1, 44100, 16) },
			{ { 'd', 'a', 't', 'a' }, createPcm16Data({ 0 }) }
		});

		WavLoader loader;
		EXPECT_THROW(loader.loadFile(filePath.string()), std::runtime_error);
}

TEST(WavLoaderTests, ThrowsWhenFmtChunkIsMissing)
{
	const auto filePath = getTestDirectory() / "wavloader_missing_fmt.wav";

	writeRiffFile(
		filePath,
		{ 'R', 'I', 'F', 'F' },
		{ 'W', 'A', 'V', 'E' },
		{
			{ { 'd', 'a', 't', 'a' }, createPcm16Data({ 0, 1 }) }
		});

		WavLoader loader;
		EXPECT_THROW(loader.loadFile(filePath.string()), std::runtime_error);
}

TEST(WavLoaderTests, ThrowsWhenDataChunkIsMissing)
{
	const auto filePath = getTestDirectory() / "wavloader_missing_data.wav";

	writeRiffFile(
		filePath,
		{ 'R', 'I', 'F', 'F' },
		{ 'W', 'A', 'V', 'E' },
		{
			{ { 'f', 'm', 't', ' ' }, createFmtChunkData(0x0001, 1, 44100, 16) }
		});

		WavLoader loader;
		EXPECT_THROW(loader.loadFile(filePath.string()), std::runtime_error);
}
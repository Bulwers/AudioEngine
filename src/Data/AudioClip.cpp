#include "AudioClip.h"

#include <stdexcept>
#include <fstream>
#include <iostream>
#include <cstring>

namespace
{
	constexpr char ClipFileMagic[4] = { 'C', 'L', 'I', 'P' };

	template<typename T>
	void writeBinary(std::ostream& stream, const T& value)
	{
		stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
		if (!stream)
		{
			throw std::runtime_error("Failed to write binary data.");
		}
	}

	template<typename T>
	T readBinary(std::istream& stream)
	{
		T value{};
		stream.read(reinterpret_cast<char*>(&value), sizeof(T));
		if (!stream)
		{
			throw std::runtime_error("Failed to read binary data.");
		}
		return value;
	}
}

AudioClip::AudioClip(std::string name, std::string filePath, std::shared_ptr<AudioBuffer> buffer)
	: name(std::move(name)), filePath(std::move(filePath)), buffer(std::move(buffer))
{
	if (!this->buffer)
	{
		throw std::invalid_argument("AudioClip buffer cannot be null.");
	}
}

AudioClip::AudioClip(
	std::string name,
	std::string filePath, 
	AudioCategory category,
	float volume,
	bool looping,
	size_t loopStartFrame,
	size_t loopEndFrame)
		:name(std::move(name)),
		filePath(std::move(filePath)),
		category(category),
		volume(volume),
		looping(looping),
		loopStartFrame(loopStartFrame),
		loopEndFrame(loopEndFrame)

{
}

void AudioClip::setLooping(bool loop, size_t startFrame, size_t endFrame)
{
	looping = loop;
	if (!loop)
	{
		loopStartFrame = 0;
		loopEndFrame = 0;
		return;
	}
	if (startFrame >= endFrame)
	{
		throw std::invalid_argument("Loop start frame cannot be greater or equal to loop end frame.");
	}
	loopStartFrame = startFrame;
	loopEndFrame = endFrame;
}

void AudioClip::saveToFile(const std::string& clipFilePath) const
{
	std::ofstream file(clipFilePath, std::ios::binary);
	if (!file)
	{
		throw std::runtime_error("Failed to open file: " + clipFilePath);
	}
	file.write(ClipFileMagic, sizeof(ClipFileMagic));
	if (!file)
	{
		throw std::runtime_error("Failed to write clip header: " + clipFilePath);
	}

	uint32_t nameLength = static_cast<uint32_t>(name.size());
	writeBinary(file, nameLength);
	file.write(name.data(), nameLength);

	uint32_t filePathLength = static_cast<uint32_t>(filePath.size());
	writeBinary(file, filePathLength);
	file.write(filePath.data(), filePathLength);

	writeBinary(file, static_cast<uint32_t>(category));
	writeBinary(file, volume);
	writeBinary(file, static_cast<uint8_t>(looping));
	writeBinary(file, static_cast<uint64_t>(loopStartFrame));
	writeBinary(file, static_cast<uint64_t>(loopEndFrame));
}

AudioClip AudioClip::loadFromFile(const std::string& clipFilePath)
{
	std::ifstream file(clipFilePath, std::ios::binary);
	if (!file)
	{
		throw std::runtime_error("Failed to open file: " + clipFilePath);
	}
	char magic[4];
	file.read(magic, sizeof(magic));
	if (!file || std::memcmp(magic, ClipFileMagic, sizeof(ClipFileMagic)) != 0)
	{
		throw std::runtime_error("Invalid clip file format: " + clipFilePath);
	}

	uint32_t nameLength = readBinary<uint32_t>(file);
	std::string name(nameLength, '\0');
	file.read(name.data(), static_cast<std::streamsize>(nameLength));
	if (!file)
	{
		throw std::runtime_error("Failed to read clip name: " + clipFilePath);
	}

	uint32_t filePathLength = readBinary<uint32_t>(file);
	std::string filePath(filePathLength, '\0');
	file.read(filePath.data(), static_cast<std::streamsize>(filePathLength));
	if (!file)
	{
		throw std::runtime_error("Failed to read clip file path: " + clipFilePath);
	}

	AudioCategory category = static_cast<AudioCategory>(readBinary<uint32_t>(file));
	float volume = readBinary<float>(file);
	bool looping = readBinary<uint8_t>(file) != 0;
	size_t loopStartFrame = static_cast<size_t>(readBinary<uint64_t>(file));
	size_t loopEndFrame = static_cast<size_t>(readBinary<uint64_t>(file));

	return AudioClip(
		std::move(name),
		std::move(filePath),
		category,
		volume,
		looping,
		loopStartFrame,
		loopEndFrame);
}

void AudioClip::printInfo() const
{
	std::cout << "\n";
	std::cout << "AudioClip Info:" << std::endl;
	std::cout << "Name: " << name << std::endl;
	std::cout << "File Path: " << filePath << std::endl;
	std::cout << "Category: " << static_cast<int>(category) << std::endl;
	std::cout << "Volume: " << volume << std::endl;
	std::cout << "Looping: " << (looping ? "Yes" : "No") << std::endl;
	std::cout << "Loop Start Frame: " << loopStartFrame << std::endl;
	std::cout << "Loop End Frame: " << loopEndFrame << std::endl;
	std::cout << "\n";
}


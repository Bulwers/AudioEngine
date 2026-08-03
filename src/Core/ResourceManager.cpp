#include "ResourceManager.h"

#include <filesystem>
#include <iostream>
#include <fstream>

template<typename T>
void write(std::ofstream& file, const T& value)
{
	file.write(reinterpret_cast<const char*>(&value), sizeof(T));
}


ResourceManager::ResourceManager()
{
	checkAudioDirectories();

}

void ResourceManager::checkAudioDirectories()
{
	std::vector<std::string> requiredDirs = {
		"Assets",
		"Assets/SoundSrc",
		"Assets/AudioClips",
		"Assets/AudioClips/Ambient",
		"Assets/AudioClips/Music",
		"Assets/AudioClips/Other",
		"Assets/AudioClips/SFX",
		"Assets/AudioClips/UI",
		"Assets/AudioClips/Voice"
	};
	const std::string soundSrcDir = "Assets/SoundSrc";
	const std::string audioClipsDir = "Assets/AudioClips";

	for (const auto& dir : requiredDirs)
	{
		if (!std::filesystem::exists(dir))
		{
			std::filesystem::create_directories(dir);
			std::cout << "Created directory: " << dir << std::endl;
		}
		else
		{
			std::cout << "Directory exists: " << dir << std::endl;
		}
	}
}

void ResourceManager::saveAudioClips()
{
}

void ResourceManager::loadAudioBuffers()
{
	audioBuffers.clear();

	for (const auto& file : std::filesystem::recursive_directory_iterator("Assets/SoundSrc"))
	{
		if (!file.is_regular_file())
			continue;

		if (file.path().extension() == ".wav")
		{
			std::string filePath = file.path().string();
			std::shared_ptr<AudioBuffer> buffer = wavLoader.loadFile(filePath);
			if (buffer)
			{
				audioBuffers[filePath] = buffer;
			}
		}
	}
}

void ResourceManager::loadAudioClips()
{
}

void ResourceManager::getAudioClips()
{
	for (const auto& pair : audioClips)
	{
		const std::string& clipName = pair.first;
		const std::shared_ptr<AudioClip>& clip = pair.second;
		std::cout << "AudioClip: " << clipName << ", FilePath: " << clip->getFilePath()
			<< ", Duration: " << clip->getDuration() << " seconds" << std::endl;
	}
}

void ResourceManager::getAudioBuffers()
{
	for (const auto& pair : audioBuffers)
	{
		const std::string& filePath = pair.first;
		const std::shared_ptr<AudioBuffer>& buffer = pair.second;
		std::cout << "AudioBuffer: " << filePath << ", Channels: " << buffer->getChannels()
			<< ", Sample Rate: " << buffer->getSampleRate() << ", Duration: " << buffer->getDuration() << " seconds" << std::endl;
	}
}
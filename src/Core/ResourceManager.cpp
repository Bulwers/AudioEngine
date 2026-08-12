#include "ResourceManager.h"
#include "ResourceManager.h"
#include "ResourceManager.h"

#include <filesystem>
#include <stdexcept>
#include <iostream>
#include <fstream>

namespace
{
	std::string normalizePath(const std::filesystem::path& path)
	{
		return path.lexically_normal().generic_string();
	}
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

void ResourceManager::createAudioClips()
{
	for (const auto& pair : audioBuffers)
	{
		const std::string& filePath = pair.first;
		const std::shared_ptr<AudioBuffer>& buffer = pair.second;
		std::string clipName = std::filesystem::path(filePath).stem().string();
		std::shared_ptr<AudioClip> clip = std::make_shared<AudioClip>(clipName, filePath, buffer);
		audioClips[clip->getName()] = clip;
	}
}

void ResourceManager::saveAudioClips()
{
	for (const auto& pair : audioClips)
	{
		const std::shared_ptr<AudioClip>& clip = pair.second;
		std::string clipFilePath = "Assets/AudioClips/" + clip->getName() + ".dat";
		clip->saveToFile(clipFilePath);
	}
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
			std::string filePath = normalizePath(file.path());
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
	audioClips.clear();

	for (const auto& file : std::filesystem::recursive_directory_iterator("Assets/AudioClips"))
	{
		if (!file.is_regular_file())
			continue;

		if (file.path().extension() == ".dat")
		{
			std::string filePath = normalizePath(file.path());
			AudioClip clip = AudioClip::loadFromFile(filePath);
			auto bufferIterator = audioBuffers.find(clip.getFilePath());	
			if (bufferIterator != audioBuffers.end())
			{
				clip.setBuffer(bufferIterator->second);
			}
			std::string name = clip.getName();
			audioClips[name] = std::make_shared<AudioClip>(std::move(clip));
		}
	}
}

std::shared_ptr<AudioBuffer> ResourceManager::getAudioBuffer(const std::string& filePath)
{
	auto it = audioBuffers.find(normalizePath(filePath));
	return it != audioBuffers.end() ? it->second : nullptr;
}

std::shared_ptr<AudioClip> ResourceManager::getAudioClip(const std::string& name)
{
	auto it = audioClips.find(name);
	return it != audioClips.end() ? it->second : nullptr;
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
#include "ResourceManager.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace
{
	std::string normalizePath(const std::filesystem::path& path)
	{
		return path.lexically_normal().generic_string();
	}

	std::string audioCategoryToFolder(AudioCategory category)
	{
		switch (category)
		{
		case AudioCategory::Ambient: return "Ambient";
		case AudioCategory::Music: return "Music";
		case AudioCategory::Other: return "Other";
		case AudioCategory::SFX: return "SFX";
		case AudioCategory::UI: return "UI";
		case AudioCategory::Voice: return "Voice";
		default: return "Other";
		}
	}
}

ResourceManager::ResourceManager(AudioResourceConfig config)
	: config(std::move(config))
{
	if (this->config.createDirectoriesOnInit)
		createAudioDirectories();
}

void ResourceManager::createAudioDirectories()
{
	const std::vector<std::filesystem::path> dirs = {
		config.rootPath,
		getSourcePath(),
		getClipPath(),
		getClipPath() / "Ambient",
		getClipPath() / "Music",
		getClipPath() / "Other",
		getClipPath() / "SFX",
		getClipPath() / "UI",
		getClipPath() / "Voice"
	};
	for (const auto& dir : dirs)
	{
		try
		{
			std::filesystem::create_directories(dir);
		}
		catch (const std::filesystem::filesystem_error& e)
		{
			std::cerr << "Error creating directory " << dir << ": " << e.what() << std::endl;
		}
	}
}

std::filesystem::path ResourceManager::getSourcePath() const
{
	return config.rootPath / config.sourceDirectory;
}

std::filesystem::path ResourceManager::getClipPath() const
{
	return config.rootPath / config.clipDirectory;
}

std::filesystem::path ResourceManager::getClipFilePath(AudioCategory category, const std::string& clipName) const
{
	return getClipPath() / audioCategoryToFolder(category) / (clipName + ".dat");
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
		clip->saveToFile(getClipFilePath(clip->getCategory(), clip->getName()).generic_string());
	}
}

void ResourceManager::loadAudioBuffers()
{
	audioBuffers.clear();

	const auto sourcePath = getSourcePath();
	if (!std::filesystem::exists(sourcePath))
		return;

	for (const auto& file : std::filesystem::recursive_directory_iterator(sourcePath))
	{
        if (!file.is_regular_file())
		{
			continue;
		}

		if (file.path().extension() == ".wav")
		{
			const std::string filePath = normalizePath(file.path());
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

	const auto clipPath = getClipPath();
	if (!std::filesystem::exists(clipPath))
		return;

	for (const auto& file : std::filesystem::recursive_directory_iterator(clipPath))
	{
        if (!file.is_regular_file())
		{
			continue;
		}

		if (file.path().extension() == ".dat")
		{
			std::string filePath = normalizePath(file.path());
			AudioClip clip = AudioClip::loadFromFile(filePath);
            const std::string clipAudioPath = normalizePath(std::filesystem::path(clip.getFilePath()));
			auto bufferIterator = audioBuffers.find(clipAudioPath);
			if (bufferIterator != audioBuffers.end())
			{
				clip.setBuffer(bufferIterator->second);
			}
			else
			{
				try
				{
					auto buffer = wavLoader.loadFile(clipAudioPath);
					if (buffer)
					{
						audioBuffers[clipAudioPath] = buffer;
						clip.setBuffer(buffer);
					}
				}
				catch (const std::exception&)
				{
					// Keep metadata loaded even if the source wav cannot be loaded right now.
				}
			}
			std::string name = clip.getName();
			audioClips[name] = std::make_shared<AudioClip>(std::move(clip));
		}
	}
}

std::vector<std::string> ResourceManager::getAvailableWavFiles() const
{
	std::vector<std::string> files;

	const auto sourcePath = getSourcePath();
	if (!std::filesystem::exists(sourcePath))
	{
		return files;
	}

	for (const auto& file : std::filesystem::recursive_directory_iterator(sourcePath))
	{
		if (!file.is_regular_file())
			continue;

		if (file.path().extension() == ".wav")
		{
			files.push_back(normalizePath(file.path()));
		}
	}

	std::sort(files.begin(), files.end());
	return files;
}

bool ResourceManager::createAndSaveAudioClip(const std::string& wavFilePath, const std::string& clipName, AudioCategory category, std::string& errorMessage)
{
	errorMessage.clear();

	if (wavFilePath.empty())
	{
		errorMessage = "No .wav file selected.";
		return false;
	}

	if (clipName.empty())
	{
		errorMessage = "Clip name cannot be empty.";
		return false;
	}

	const std::string normalizedPath = normalizePath(std::filesystem::path(wavFilePath));

	std::shared_ptr<AudioBuffer> buffer;
	try
	{
		buffer = wavLoader.loadFile(normalizedPath);
	}
	catch (const std::exception& ex)
	{
		errorMessage = ex.what();
		return false;
	}

	if (!buffer)
	{
		errorMessage = "Failed to load selected .wav file.";
		return false;
	}

	audioBuffers[normalizedPath] = buffer;

	auto clip = std::make_shared<AudioClip>(clipName, normalizedPath, buffer);
	clip->setCategory(category);
	audioClips[clipName] = clip;

	try
	{
		clip->saveToFile(getClipFilePath(category, clipName).generic_string());
	}
	catch (const std::exception& ex)
	{
		errorMessage = ex.what();
		return false;
	}

	return true;
}

bool ResourceManager::removeAudioClipFile(const std::string& clipName, std::string& errorMessage)
{
	errorMessage.clear();

	if (clipName.empty())
	{
		errorMessage = "Clip name cannot be empty.";
		return false;
	}

	auto clipIt = audioClips.find(clipName);
	if (clipIt == audioClips.end() || !clipIt->second)
	{
		errorMessage = "Clip not found.";
		return false;
	}

	const AudioCategory category = clipIt->second->getCategory();
	const auto clipFilePath = getClipFilePath(category, clipName);

	std::error_code ec;
	const bool removed = std::filesystem::remove(clipFilePath, ec);
	if (!removed)
	{
		if (ec)
		{
			errorMessage = "Failed to remove clip file: " + ec.message();
		}
		else
		{
			errorMessage = "Clip file does not exist: " + clipFilePath.generic_string();
		}
		return false;
	}

	audioClips.erase(clipIt);
	return true;
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

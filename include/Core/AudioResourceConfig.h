#pragma once

#include <filesystem>

// Paths are resolved relative to the process working directory unless rootPath is absolute.
struct AudioResourceConfig
{
	std::filesystem::path rootPath = "Assets";
	std::filesystem::path sourceDirectory = "SoundSrc";
	std::filesystem::path clipDirectory = "AudioClips";
	bool createDirectoriesOnInit = true;
};

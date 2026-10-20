#pragma once

#include <functional>
#include <string>
#include <vector>

class ResourceManager;

class ResourcePanel
{
public:

	explicit ResourcePanel(ResourceManager& resourceManager, std::function<void(const std::string&)> onSourceCreated);

	void render();

private:

	ResourceManager& resourceManager;
	std::function<void(const std::string&)> onSourceCreated;
	std::string selectedClipName;
	std::vector<std::string> availableWavFiles;
	int selectedWavFileIndex = -1;
 bool wavFilesInitialized = false;
	char newClipName[128] = "";
	int selectedCategoryIndex = 0;
};
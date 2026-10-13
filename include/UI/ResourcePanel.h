#pragma once

#include <functional>
#include <string>

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
};
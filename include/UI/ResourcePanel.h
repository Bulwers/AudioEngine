#pragma once

class ResourceManager;

class ResourcePanel
{
public:

	explicit ResourcePanel(ResourceManager& resourceManager);

	void render();

private:

	ResourceManager& resourceManager;
};
#pragma once

#include <memory>

class SourcePanel
{
public:

	void render();
	void setSelectedSource(std::shared_ptr<class AudioSource> source) { selectedSource = source; }

private:

	std::shared_ptr<class AudioSource> selectedSource;
};
#pragma once

#include <cstdint>

struct AudioFormat
{
	uint32_t sampleRate = 44100;
	uint16_t channels = 2;
};

#include <gtest/gtest.h>

#include "AudioBuffer.h"

TEST(AudioBufferTests, DefaultBufferHasNoSamples)
{
	AudioBuffer buffer;

	EXPECT_EQ(buffer.getSampleCount(), 0u);
	EXPECT_EQ(buffer.getFrameCount(), 0u);
	EXPECT_DOUBLE_EQ(buffer.getDuration(), 0.0);
}
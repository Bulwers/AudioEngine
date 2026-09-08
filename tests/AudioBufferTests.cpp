#include <gtest/gtest.h>

#include "AudioBuffer.h"

TEST(AudioBufferTests, DefaultBufferHasNoSamples)
{
	AudioBuffer buffer;

	EXPECT_EQ(buffer.getSampleCount(), 0u);
	EXPECT_EQ(buffer.getFrameCount(), 0u);
	EXPECT_DOUBLE_EQ(buffer.getDuration(), 0.0);
}

TEST(AudioBufferTests, BufferWithData)
{
	AudioBuffer buffer("test.wav", 2, 44100, 16, std::vector<float>{0.0f, 0.1f, 0.2f, 0.3f});

	EXPECT_EQ(buffer.getSampleCount(), 4u);
	EXPECT_EQ(buffer.getFrameCount(), 2u);
	EXPECT_DOUBLE_EQ(buffer.getDuration(), static_cast<double>(buffer.getFrameCount() / 44100.0));
	EXPECT_EQ(buffer.getSample(0, 1), 0.1f);
	EXPECT_EQ(buffer.getSample(1, 1), 0.3f);
	EXPECT_EQ(buffer.getFilePath(), "test.wav");
	EXPECT_ANY_THROW(buffer.getSample(2, 0)); // Out of bounds

}
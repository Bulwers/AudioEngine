#include <gtest/gtest.h>

#include "AudioMixer.h"
#include "AudioSource.h"
#include "AudioClip.h"
#include "AudioBuffer.h"

#include <memory>
#include <vector>

namespace
{
	std::shared_ptr<AudioSource> createSource(const std::vector<float>& samples, uint16_t channels = 1)
	{
		auto buffer = std::make_shared<AudioBuffer>("test.wav", channels, 1, 16, samples);
		auto clip = std::make_shared<AudioClip>("TestClip", "test.wav", buffer);
		auto source = std::make_shared<AudioSource>(clip);
		source->play();
		return source;
	}
}

TEST(AudioMixerTests, MixesSingleMonoSource)
{
	AudioMixer mixer;
	auto source = createSource({ 0.25f, 0.5f, -0.25f });

	mixer.addSource(source);

	float output[3] = { 99.0f, 99.0f, 99.0f };
	mixer.mixAudio(output, 3, 1);

	EXPECT_FLOAT_EQ(output[0], 0.25f);
	EXPECT_FLOAT_EQ(output[1], 0.5f);
	EXPECT_FLOAT_EQ(output[2], -0.25f);
	EXPECT_DOUBLE_EQ(source->getPlaybackPosition(), 3.0);
}

TEST(AudioMixerTests, SumsMultipleSources)
{
	AudioMixer mixer;
	auto sourceA = createSource({ 0.25f, 0.25f });
	auto sourceB = createSource({ 0.5f, -0.25f });

	mixer.addSource(sourceA);
	mixer.addSource(sourceB);

	float output[2] = { 0.0f, 0.0f };
	mixer.mixAudio(output, 2, 1);

	EXPECT_FLOAT_EQ(output[0], 0.75f);
	EXPECT_FLOAT_EQ(output[1], 0.0f);
}

TEST(AudioMixerTests, StopsSourceAtEndWhenNotLooping)
{
	AudioMixer mixer;
	auto source = createSource({ 1.0f, 2.0f });

	mixer.addSource(source);

	float output[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	mixer.mixAudio(output, 4, 1);

	EXPECT_FLOAT_EQ(output[0], 1.0f);
	EXPECT_FLOAT_EQ(output[1], 1.0f);
	EXPECT_FLOAT_EQ(output[2], 0.0f);
	EXPECT_FLOAT_EQ(output[3], 0.0f);
	EXPECT_FALSE(source->isPlaying());
}

TEST(AudioMixerTests, LoopsSourceWhenLoopingIsEnabled)
{
	AudioMixer mixer;
	auto source = createSource({ 1.0f, 0.4f });
	source->setLooping(true);

	mixer.addSource(source);

	float output[5] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	mixer.mixAudio(output, 5, 1);

	EXPECT_FLOAT_EQ(output[0], 1.0f);
	EXPECT_FLOAT_EQ(output[1], 0.4f);
	EXPECT_FLOAT_EQ(output[2], 1.0f);
	EXPECT_FLOAT_EQ(output[3], 0.4f);
	EXPECT_FLOAT_EQ(output[4], 1.0f);
	EXPECT_TRUE(source->isPlaying());
}

TEST(AudioMixerTests, ClearsOutputBufferBeforeMixing)
{
	AudioMixer mixer;
	auto source = createSource({ 0.5f, 0.25f });

	mixer.addSource(source);

	float output[2] = { 10.0f, 10.0f };
	mixer.mixAudio(output, 2, 1);

	EXPECT_FLOAT_EQ(output[0], 0.5f);
	EXPECT_FLOAT_EQ(output[1], 0.25f);
}
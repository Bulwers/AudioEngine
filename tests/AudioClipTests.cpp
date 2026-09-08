#include <gtest/gtest.h>

#include "AudioClip.h"
#include "AudioBuffer.h"
#include <iostream>

TEST(AudioClipTests, AudioClipDataTests)
{
	AudioBuffer buffer("path/to/clip.wav", 2, 44100, 16, std::vector<float>{0.0f, 0.1f, 0.2f, 0.3f});
	AudioClip clip("Clip", "path/to/clip.wav", std::make_shared<AudioBuffer>(buffer));

	EXPECT_EQ(clip.getName(), "Clip");
	EXPECT_EQ(clip.getFilePath(), "path/to/clip.wav");
	clip.setName("NewClip");
	EXPECT_EQ(clip.getName(), "NewClip");

	clip.setCategory(AudioCategory::SFX);
	EXPECT_EQ(clip.getCategory(), AudioCategory::SFX);

	EXPECT_EQ(clip.getVolume(), 1.0f);
	clip.setVolume(0.5f);
	EXPECT_EQ(clip.getVolume(), 0.5f);

	clip.setLooping(true, 0, 1);
	EXPECT_EQ(clip.isLooping(), true);

}

TEST(AudioClipTests, SaveAndLoadTests)
{
	AudioBuffer buffer("path/to/clip.wav", 1, 44100, 8, std::vector<float>{0.0f, 0.1f, 0.2f, 0.3f});
	AudioClip clip("Clip", "path/to/clip.wav", std::make_shared<AudioBuffer>(buffer));

	clip.saveToFile("clip.dat");
	clip.printInfo();

	clip.setCategory(AudioCategory::Music);
	clip.setLooping(false);
	clip.setVolume(3.0f);
	clip.setName("NewClip");

	clip.saveToFile("newClip.dat");

	AudioClip loadedClip = AudioClip::loadFromFile("clip.dat");
	loadedClip.printInfo();

	AudioClip loadedNewClip = AudioClip::loadFromFile("newClip.dat");
	loadedNewClip.printInfo();

}

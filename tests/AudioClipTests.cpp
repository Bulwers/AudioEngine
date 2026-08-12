#include <gtest/gtest.h>

#include "AudioClip.h"
#include "AudioBuffer.h"
#include <iostream>

TEST(AudioClipTests, SaveAndLoadTests)
{
	AudioBuffer buffer1("path/to/clip1.wav", 2, 44100, 16, std::vector<float>{0.0f, 0.1f, 0.2f, 0.3f});
	AudioBuffer buffer2("path/to/clip2.wav", 1, 22050, 8, std::vector<float>{0.4f, 0.5f});
	AudioBuffer buffer3("path/to/clip3.wav", 2, 48000, 24, std::vector<float>{0.6f, 0.7f, 0.8f, 0.9f});

	AudioClip clip1("Clip1", "path/to/clip1.wav", std::make_shared<AudioBuffer>(buffer1));
	AudioClip clip2("Clip2", "path/to/clip2.wav", std::make_shared<AudioBuffer>(buffer2));
	AudioClip clip3("Clip3", "path/to/clip3.wav", std::make_shared<AudioBuffer>(buffer3));

	clip1.saveToFile("clip1.dat");
	clip2.saveToFile("clip2.dat");
	clip3.saveToFile("clip3.dat");

	AudioClip loadedClip1 = AudioClip::loadFromFile("clip1.dat");
	AudioClip loadedClip2 = AudioClip::loadFromFile("clip2.dat");
	AudioClip loadedClip3 = AudioClip::loadFromFile("clip3.dat");

}
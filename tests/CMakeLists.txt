
add_executable(AudioEngineTests
	AudioBufferTests.cpp
	AudioClipTests.cpp
	WavLoaderTests.cpp
	AudioMixerTests.cpp
	ResourceManagerTests.cpp)

target_link_libraries(AudioEngineTests PRIVATE
	AudioEngineCore
	gtest_main
)

include(GoogleTest)
gtest_discover_tests(AudioEngineTests)
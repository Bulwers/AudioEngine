cmake_minimum_required(VERSION 3.20)

project(AudioEngine)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

include(CTest) # enable_testing() BUILD_TESTING

option(AUDIO_BUILD_EDITOR "Build the audio engine editor" ON)
option(AUDIO_BUILD_SDL_BACKEND "Build the SDL3 audio backend" ON)

# AudioEngineRuntime library, without SDL and ImGui

add_library(AudioEngineRuntime STATIC
	src/Core/AudioEngine.cpp
	src/Data/AudioBuffer.cpp
	src/Data/AudioClip.cpp
	src/Playback/AudioSource.cpp
	src/Core/AudioMixer.cpp
	src/Core/WavLoader.cpp
	src/Core/ResourceManager.cpp
)

target_include_directories(AudioEngineRuntime PUBLIC
	include/Data
	include/Playback
	include/Core
	include/DSP
)

target_link_libraries(AudioEngineRuntime PUBLIC Threads::Threads)

# SDL3 backend
if(AUDIO_BUILD_EDITOR OR AUDIO_BUILD_SDL_BACKEND)
	set(SDL_SHARED OFF CACHE BOOL "" FORCE)
	set(SDL_STATIC ON CACHE BOOL "" FORCE)
	add_subdirectory(third_party/SDL)

	add_library(AudioBackendSDL3 STATIC
		src/Playback/AudioDevice.cpp
	)

	target_link_libraries(AudioBackendSDL3 PUBLIC
		AudioEngineRuntime
		SDL3::SDL3
	)
endif()

if(AUDIO_BUILD_EDITOR)
	# ImGui
	add_library(imgui
		third_party/imgui/imgui.cpp
		third_party/imgui/imgui_draw.cpp
		third_party/imgui/imgui_tables.cpp
		third_party/imgui/imgui_widgets.cpp
		third_party/imgui/imgui_demo.cpp

		third_party/imgui/backends/imgui_impl_sdl3.cpp
		third_party/imgui/backends/imgui_impl_sdlrenderer3.cpp
	)

	target_include_directories(imgui PUBLIC
		third_party/imgui
		third_party/imgui/backends
	)

	target_link_libraries(imgui PUBLIC SDL3::SDL3)
	
	# Editor
	add_library(AudioEngineEditor STATIC
		src/UI/EditorUI.cpp
		src/UI/AudioSourcesPanel.cpp
		src/UI/ResourcePanel.cpp

		src/Tick/ITickable.cpp
		src/Tick/TickSystem.cpp
	)

	target_include_directories(AudioEngineEditor PUBLIC
		include/UI
		include/Core
		include/Data
		include/Playback
		include/DSP
		include/Tick
	)

	target_link_libraries(AudioEngineEditor
		PRIVATE imgui
		PUBLIC AudioEngineRuntime
	)

	# Application
	add_executable(AudioEngine WIN32
		src/main.cpp
	)
	
	if (MSVC)
		target_link_options(AudioEngine PRIVATE "/ENTRY:mainCRTStartup")
	endif()
	
	target_link_libraries(AudioEngine PRIVATE
		AudioEngineEditor
		AudioBackendSDL3
		SDL3::SDL3
		imgui
	)
endif()

if(BUILD_TESTING)
	set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
	enable_testing()
	add_subdirectory(third_party/googletest)
	add_subdirectory(tests)
endif()

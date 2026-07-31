cmake_minimum_required(VERSION 3.20)

project(AudioEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

enable_testing()

add_subdirectory(third_party/SDL)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/googletest)

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

add_library(AudioEngineCore
	src/Core/WavLoader.cpp
	src/Data/AudioBuffer.cpp
	src/Data/AudioClip.cpp
)

target_include_directories(AudioEngineCore PUBLIC
	include/Core
	include/Data
)

add_executable(AudioEngine
	src/main.cpp
)

target_link_libraries(AudioEngine PRIVATE
	AudioEngineCore
	SDL3::SDL3
	imgui
)

add_subdirectory(tests)
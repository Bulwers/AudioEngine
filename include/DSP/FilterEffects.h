#pragma once

#include "IAudioProcessor.h"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <complex>

// These first-order filters keep per-channel state and support up to 16 channels.
class LowPassFilter final : public IAudioProcessor
{
public:
	void setCutoffFrequency(float frequencyHz) noexcept;
	float getCutoffFrequency() const noexcept { return cutoffFrequency.load(std::memory_order_relaxed); }

	void process(float* samples, std::size_t frameCount,
		std::uint16_t channelCount, std::uint32_t sampleRate) noexcept override;

private:
	static constexpr std::size_t MaxChannels = 16;
	std::atomic<float> cutoffFrequency = 1200.0f;
	std::array<float, MaxChannels> lowPassState{};
	float currentAlpha = 0.0f;
	bool initialized = false;
};

class HighPassFilter final : public IAudioProcessor
{
public:
	void setCutoffFrequency(float frequencyHz) noexcept;
	float getCutoffFrequency() const noexcept { return cutoffFrequency.load(std::memory_order_relaxed); }

	void process(float* samples, std::size_t frameCount,
		std::uint16_t channelCount, std::uint32_t sampleRate) noexcept override;

private:
	static constexpr std::size_t MaxChannels = 16;
	std::atomic<float> cutoffFrequency = 80.0f;
	std::array<float, MaxChannels> lowPassState{};
	float currentAlpha = 0.0f;
	bool initialized = false;
};

struct EqualizerBandSettings
{
	float frequencyHz = 1000.0f;
	float gainDb = 0.0f;
	float q = 1.0f;
};

// Eight-band parametric equalizer using peaking biquad filters.
class ParametricEqualizer final : public IAudioProcessor
{
public:
	static constexpr std::size_t MaxBands = 8;
	static constexpr std::size_t DefaultBandCount = 3;
	ParametricEqualizer() noexcept;

	std::size_t getBandCount() const noexcept { return bandCount.load(std::memory_order_acquire); }
	EqualizerBandSettings getBand(std::size_t index) const noexcept;
	bool addBand(EqualizerBandSettings settings) noexcept;
	bool removeBand(std::size_t index) noexcept;
	void setBand(std::size_t index, EqualizerBandSettings settings) noexcept;
	float getFrequencyResponseDb(float frequencyHz, std::uint32_t sampleRate) const noexcept;

	void process(float* samples, std::size_t frameCount,
		std::uint16_t channelCount, std::uint32_t sampleRate) noexcept override;

private:
	static constexpr std::size_t MaxChannels = 16;
	struct AtomicBand
	{
		std::atomic<float> frequencyHz{ 1000.0f };
		std::atomic<float> gainDb{ 0.0f };
		std::atomic<float> q{ 1.0f };
	};
	struct RuntimeBand
	{
		float frequencyHz = 1000.0f;
		float gainDb = 0.0f;
		float q = 1.0f;
		float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;
		float a1 = 0.0f, a2 = 0.0f;
		std::array<float, MaxChannels> z1{};
		std::array<float, MaxChannels> z2{};
		bool initialized = false;
	};
	static constexpr std::array<EqualizerBandSettings, DefaultBandCount> DefaultBands = {{
		{ 100.0f, 0.0f, 1.0f }, { 1000.0f, 0.0f, 1.0f }, { 8000.0f, 0.0f, 1.0f }
	}};
	std::array<AtomicBand, MaxBands> bands{};
	std::array<RuntimeBand, MaxBands> runtimeBands{};
	std::atomic<std::size_t> bandCount{ DefaultBandCount };
};

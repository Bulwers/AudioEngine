#include "FilterEffects.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr float MinCutoffHz = 10.0f;
	constexpr float MaxCutoffHz = 24000.0f;
	constexpr float MaxGainDb = 24.0f;
	constexpr float SmoothingTimeSeconds = 0.05f;
	constexpr float TwoPi = 6.2831853071795864769f;

	float clampCutoff(float frequencyHz, std::uint32_t sampleRate) noexcept
	{
		const float nyquistLimit = std::max(1.0f, static_cast<float>(sampleRate) * 0.49f);
		return std::clamp(frequencyHz, std::min(MinCutoffHz, nyquistLimit),
			std::min(MaxCutoffHz, nyquistLimit));
	}

	float calculateAlpha(float cutoffHz, std::uint32_t sampleRate) noexcept
	{
		return 1.0f - std::exp(-TwoPi * cutoffHz / static_cast<float>(sampleRate));
	}

	float blockSmoothingFactor(std::size_t frameCount, std::uint32_t sampleRate) noexcept
	{
		const float elapsedSeconds = static_cast<float>(frameCount) / static_cast<float>(sampleRate);
		return 1.0f - std::exp(-elapsedSeconds / SmoothingTimeSeconds);
	}

	float smooth(float current, float target, float factor) noexcept
	{
		return current + (target - current) * factor;
	}

	float clampFrequency(float frequencyHz, std::uint32_t sampleRate) noexcept
	{
		const float nyquistLimit = std::max(1.0f, static_cast<float>(sampleRate) * 0.49f);
		return std::clamp(frequencyHz, std::min(10.0f, nyquistLimit), nyquistLimit);
	}

	float clampQ(float q) noexcept
	{
		return std::clamp(q, 0.1f, 10.0f);
	}

	void calculatePeakingCoefficients(float frequencyHz, float gainDb, float q,
		std::uint32_t sampleRate, float& b0, float& b1, float& b2, float& a1, float& a2) noexcept
	{
		const float omega = TwoPi * frequencyHz / static_cast<float>(sampleRate);
		const float cosine = std::cos(omega);
		const float alpha = std::sin(omega) / (2.0f * q);
		const float amplitude = std::pow(10.0f, gainDb / 40.0f);
		const float a0 = 1.0f + alpha / amplitude;
		b0 = (1.0f + alpha * amplitude) / a0;
		b1 = (-2.0f * cosine) / a0;
		b2 = (1.0f - alpha * amplitude) / a0;
		a1 = (-2.0f * cosine) / a0;
		a2 = (1.0f - alpha / amplitude) / a0;
	}

	float removeDenormal(float value) noexcept
	{
		return std::abs(value) < 1.0e-20f ? 0.0f : value;
	}

}

void LowPassFilter::setCutoffFrequency(float frequencyHz) noexcept
{
	if (std::isfinite(frequencyHz))
		cutoffFrequency.store(std::clamp(frequencyHz, MinCutoffHz, MaxCutoffHz), std::memory_order_relaxed);
}

void LowPassFilter::process(float* samples, std::size_t frameCount,
	std::uint16_t channelCount, std::uint32_t sampleRate) noexcept
{
	if (!samples || frameCount == 0 || channelCount == 0 || sampleRate == 0)
		return;

	const float targetAlpha = calculateAlpha(clampCutoff(cutoffFrequency.load(std::memory_order_relaxed), sampleRate), sampleRate);
	if (!initialized)
	{
		currentAlpha = targetAlpha;
		initialized = true;
	}
	else
	{
		currentAlpha = smooth(currentAlpha, targetAlpha, blockSmoothingFactor(frameCount, sampleRate));
	}

	const std::size_t channelsToProcess = std::min<std::size_t>(channelCount, MaxChannels);
	for (std::size_t frame = 0; frame < frameCount; ++frame)
	{
		for (std::size_t channel = 0; channel < channelsToProcess; ++channel)
		{
			const std::size_t index = frame * channelCount + channel;
			float& state = lowPassState[channel];
			state = removeDenormal(state + currentAlpha * (samples[index] - state));
			samples[index] = state;
		}
	}
}

void HighPassFilter::setCutoffFrequency(float frequencyHz) noexcept
{
	if (std::isfinite(frequencyHz))
		cutoffFrequency.store(std::clamp(frequencyHz, MinCutoffHz, MaxCutoffHz), std::memory_order_relaxed);
}

void HighPassFilter::process(float* samples, std::size_t frameCount,
	std::uint16_t channelCount, std::uint32_t sampleRate) noexcept
{
	if (!samples || frameCount == 0 || channelCount == 0 || sampleRate == 0)
		return;

	const float targetAlpha = calculateAlpha(clampCutoff(cutoffFrequency.load(std::memory_order_relaxed), sampleRate), sampleRate);
	if (!initialized)
	{
		currentAlpha = targetAlpha;
		initialized = true;
	}
	else
	{
		currentAlpha = smooth(currentAlpha, targetAlpha, blockSmoothingFactor(frameCount, sampleRate));
	}

	const std::size_t channelsToProcess = std::min<std::size_t>(channelCount, MaxChannels);
	for (std::size_t frame = 0; frame < frameCount; ++frame)
	{
		for (std::size_t channel = 0; channel < channelsToProcess; ++channel)
		{
			const std::size_t index = frame * channelCount + channel;
			const float input = samples[index];
			float& state = lowPassState[channel];
			state = removeDenormal(state + currentAlpha * (input - state));
			samples[index] = input - state;
		}
	}
}

ParametricEqualizer::ParametricEqualizer() noexcept
{
	for (std::size_t i = 0; i < DefaultBands.size(); ++i)
		setBand(i, DefaultBands[i]);
}

EqualizerBandSettings ParametricEqualizer::getBand(std::size_t index) const noexcept
{
	if (index >= MaxBands)
		return {};
	return {
		bands[index].frequencyHz.load(std::memory_order_relaxed),
		bands[index].gainDb.load(std::memory_order_relaxed),
		bands[index].q.load(std::memory_order_relaxed)
	};
}

void ParametricEqualizer::setBand(std::size_t index, EqualizerBandSettings settings) noexcept
{
	if (index >= MaxBands)
		return;
	if (std::isfinite(settings.frequencyHz))
		bands[index].frequencyHz.store(std::clamp(settings.frequencyHz, 10.0f, 24000.0f), std::memory_order_relaxed);
	if (std::isfinite(settings.gainDb))
		bands[index].gainDb.store(std::clamp(settings.gainDb, -24.0f, 24.0f), std::memory_order_relaxed);
	if (std::isfinite(settings.q))
		bands[index].q.store(clampQ(settings.q), std::memory_order_relaxed);
}

	bool ParametricEqualizer::addBand(EqualizerBandSettings settings) noexcept
	{
		const std::size_t count = bandCount.load(std::memory_order_relaxed);
		if (count >= MaxBands)
			return false;
		setBand(count, settings);
		bandCount.store(count + 1, std::memory_order_release);
		return true;
	}

	bool ParametricEqualizer::removeBand(std::size_t index) noexcept
	{
		const std::size_t count = bandCount.load(std::memory_order_relaxed);
		if (index >= count || count <= 1)
			return false;
		for (std::size_t i = index; i + 1 < count; ++i)
			setBand(i, getBand(i + 1));
		bandCount.store(count - 1, std::memory_order_release);
		return true;
	}

	float ParametricEqualizer::getFrequencyResponseDb(float frequencyHz, std::uint32_t sampleRate) const noexcept
	{
		if (!std::isfinite(frequencyHz) || sampleRate == 0)
			return 0.0f;

		const float nyquistLimit = std::max(1.0f, static_cast<float>(sampleRate) * 0.49f);
		const float omega = TwoPi * std::clamp(frequencyHz, 0.0f, nyquistLimit) / static_cast<float>(sampleRate);
		const std::complex<float> z1(std::cos(omega), -std::sin(omega));
		const std::complex<float> z2 = z1 * z1;
		std::complex<float> response(1.0f, 0.0f);
		const std::size_t count = bandCount.load(std::memory_order_acquire);
		for (std::size_t i = 0; i < count; ++i)
		{
			const auto settings = getBand(i);
			float b0, b1, b2, a1, a2;
			calculatePeakingCoefficients(clampFrequency(settings.frequencyHz, sampleRate), settings.gainDb,
				clampQ(settings.q), sampleRate, b0, b1, b2, a1, a2);
			const std::complex<float> numerator = b0 + b1 * z1 + b2 * z2;
			const std::complex<float> denominator = 1.0f + a1 * z1 + a2 * z2;
			response *= numerator / denominator;
		}
		return 20.0f * std::log10(std::max(std::abs(response), 1.0e-6f));
}

void ParametricEqualizer::process(float* samples, std::size_t frameCount,
	std::uint16_t channelCount, std::uint32_t sampleRate) noexcept
{
	if (!samples || frameCount == 0 || channelCount == 0 || sampleRate == 0)
		return;

	const float factor = blockSmoothingFactor(frameCount, sampleRate);
	const std::size_t count = bandCount.load(std::memory_order_acquire);
	const std::size_t channelsToProcess = std::min<std::size_t>(channelCount, MaxChannels);
	for (std::size_t bandIndex = 0; bandIndex < count; ++bandIndex)
	{
		auto& runtime = runtimeBands[bandIndex];
		const auto settings = getBand(bandIndex);
		const float targetFrequency = clampFrequency(settings.frequencyHz, sampleRate);
		const float targetGain = settings.gainDb;
		const float targetQ = clampQ(settings.q);
		if (!runtime.initialized)
		{
			runtime.frequencyHz = targetFrequency;
			runtime.gainDb = targetGain;
			runtime.q = targetQ;
			runtime.initialized = true;
		}
		else
		{
			runtime.frequencyHz = smooth(runtime.frequencyHz, targetFrequency, factor);
			runtime.gainDb = smooth(runtime.gainDb, targetGain, factor);
			runtime.q = smooth(runtime.q, targetQ, factor);
		}
		calculatePeakingCoefficients(runtime.frequencyHz, runtime.gainDb, runtime.q, sampleRate,
			runtime.b0, runtime.b1, runtime.b2, runtime.a1, runtime.a2);
		for (std::size_t frame = 0; frame < frameCount; ++frame)
		{
			for (std::size_t channel = 0; channel < channelsToProcess; ++channel)
			{
				const std::size_t index = frame * channelCount + channel;
				const float input = samples[index];
				const float output = runtime.b0 * input + runtime.z1[channel];
				runtime.z1[channel] = removeDenormal(runtime.b1 * input - runtime.a1 * output + runtime.z2[channel]);
				runtime.z2[channel] = removeDenormal(runtime.b2 * input - runtime.a2 * output);
				samples[index] = output;
			}
		}
	}
}

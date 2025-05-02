#include "delay.h"
#include <cmath>
#include <algorithm>  // for std::clamp

Delay::Delay() : sampleRate(44100.0), maxDelayTime(2.0), modPhase(0.0f), writeIndex(0) {
  setSampleRate(sampleRate);
}

void Delay::setSampleRate(double newSampleRate) {
  sampleRate = newSampleRate;
  size_t bufferSize = static_cast<size_t>(sampleRate * maxDelayTime) + 1;

  for (auto& buf : delayBuffer)
    buf.assign(bufferSize, 0.0f);

  fadeInIncrement = 1.0f / static_cast<float>(sampleRate * 0.02);  // 20ms fade-in
  fadeInAmount = 0.0f;
}

void Delay::setParameters(const Parameters& params) {
  feedback = std::clamp(params.feedback, 0.0f, 0.99f);
  wetLevel = params.wetLevel;
  dryLevel = params.dryLevel;
  modDepth = params.modulationDepthSeconds;
  modRate = params.modulationRateHz;
  mode = params.mode;

  // Set delay time based on tempo sync or manual time input
  float newDelayTime =
      params.syncToTempo ? (60.0f / params.hostBpm) * params.noteDivision : params.delayTimeSeconds;

  if (std::abs(delayTimeSeconds - newDelayTime) > 0.0001f) {
    delayTimeSeconds = newDelayTime;
    samplesUntilNextFlip = static_cast<size_t>(delayTimeSeconds * sampleRate);
  }
}

void Delay::processMono(float* samples, int numSamples) {
  auto& buf = delayBuffer[0];

  for (int i = 0; i < numSamples; ++i) {
    float mod = std::sin(modPhase * juce::MathConstants<float>::twoPi) * (modDepth / 100);
    size_t delaySamples = static_cast<size_t>((delayTimeSeconds + mod) * sampleRate);
    size_t readIndex = (writeIndex + buf.size() - delaySamples) % buf.size();

    float delayed = buf[readIndex];
    float input = samples[i];
    float wet = delayed;
    float output = dryLevel * input + wetLevel * wet;

    samples[i] = output;
    buf[writeIndex] = input + delayed * feedback;

    writeIndex = (writeIndex + 1) % buf.size();
    modPhase = std::fmod(modPhase + modRate / static_cast<float>(sampleRate), 1.0f);
  }
}

void Delay::processStereo(float* left, float* right, int numSamples) {
  auto& bufL = delayBuffer[0];
  auto& bufR = delayBuffer[1];
  const auto bufferSize = bufL.size();

  for (int i = 0; i < numSamples; ++i) {
    float mod = std::sin(modPhase * juce::MathConstants<float>::twoPi) * (modDepth / 100.0f);
    float delaySamples = static_cast<float>((delayTimeSeconds + mod) * sampleRate);
    delaySamples = std::clamp(delaySamples, 1.0f, static_cast<float>(bufferSize - 2));

    // Interpolate between two samples in the delay buffer for smoother repeats
    float readPos = static_cast<float>(writeIndex) + bufferSize - delaySamples;
    while (readPos < 0.0f)
      readPos += bufferSize;
    size_t i0 = static_cast<size_t>(readPos) % bufferSize;
    size_t i1 = (i0 + 1) % bufferSize;
    float frac = readPos - std::floor(readPos);

    // Linear interpolation
    float delayedL = bufL[i0] * (1.0f - frac) + bufL[i1] * frac;
    float delayedR = bufR[i0] * (1.0f - frac) + bufR[i1] * frac;

    float inL = left[i];
    float inR = right[i];

    // Soften the attack of the delay using a fade-in
    float fadeFactor = std::min(1.0f, fadeInAmount);
    fadeInAmount += fadeInIncrement;

    if (mode == DelayMode::PingPong) {
      left[i] = dryLevel * inL + wetLevel * delayedL * fadeFactor;
      right[i] = dryLevel * inR + wetLevel * delayedR * fadeFactor;

      // Inject input from one channel to start the ping-pong chain
      bufL[writeIndex] = inL * (1.0f - feedback);
      bufR[writeIndex] = 0.0f;

      // Cross-feed feedback
      if (pingPongFlip) {
        bufR[writeIndex] += delayedL * feedback;
      } else {
        bufL[writeIndex] += delayedR * feedback;
      }

      // Flip once per full repeat/delay time
      if (--samplesUntilNextFlip <= 0) {
        pingPongFlip = !pingPongFlip;
        samplesUntilNextFlip = static_cast<size_t>(delaySamples);
      }
    } else {
      left[i] = dryLevel * inL + wetLevel * delayedL * fadeFactor;
      right[i] = dryLevel * inR + wetLevel * delayedR * fadeFactor;

      bufL[writeIndex] = inL + delayedL * feedback;
      bufR[writeIndex] = inR + delayedR * feedback;
    }

    writeIndex = (writeIndex + 1) % bufferSize;
    modPhase = std::fmod(modPhase + modRate / static_cast<float>(sampleRate), 1.0f);
  }
}

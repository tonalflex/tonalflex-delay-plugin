#pragma once

#include <vector>
#include <cmath>
#include <juce_audio_processors/juce_audio_processors.h>

class Delay {
public:
  enum class DelayMode { Mono, Stereo, PingPong };

  struct Parameters {
    float delayTimeSeconds = 0.5f;
    float feedback = 0.5f;
    float wetLevel = 0.5f;
    float dryLevel = 0.5f;

    float modulationDepthSeconds = 0.002f;
    float modulationRateHz = 0.25f;

    bool syncToTempo = false;
    float hostBpm = 120.0f;
    float noteDivision = 0.25f;  // quarter note = 0.25, eighth = 0.125, dotted eighth = 0.1875

    DelayMode mode = DelayMode::Stereo;
  };

  Delay() : sampleRate(44100.0), maxDelayTime(2.0), modPhase(0.0f), writeIndex(0) {
    setSampleRate(sampleRate);
  }

  void setSampleRate(double newSampleRate) {
    sampleRate = newSampleRate;
    size_t bufferSize = static_cast<size_t>(sampleRate * maxDelayTime) + 1;

    for (auto& buf : delayBuffer)
      buf.assign(bufferSize, 0.0f);

    // Set the fade-in increment based on the sample rate
    fadeInIncrement = 1.0f / (sampleRate * 0.02f);  // 20ms fade-in for smooth attack
    fadeInAmount = 0.0f;                            // reset on sample rate change
  }

  void setParameters(const Parameters& params) {
    feedback = std::clamp(params.feedback, 0.0f, 0.99f);
    wetLevel = params.wetLevel;
    dryLevel = params.dryLevel;
    modDepth = params.modulationDepthSeconds;
    modRate = params.modulationRateHz;
    mode = params.mode;

    // Set delay time based on tempo sync or manual time input
    float newDelayTime = params.syncToTempo ? (60.0f / params.hostBpm) * params.noteDivision
                                            : params.delayTimeSeconds;

    if (std::abs(delayTimeSeconds - newDelayTime) > 0.0001f) {
      delayTimeSeconds = newDelayTime;
      // Calculate number of samples in one delay repeat
      samplesUntilNextFlip = static_cast<size_t>(delayTimeSeconds * sampleRate);
    }
  }

  // Mono Output
  void processMono(float* samples, int numSamples) {
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
      modPhase = std::fmod(modPhase + modRate / sampleRate, 1.0f);
    }
  }

  // Stereo Output
  void processStereo(float* left, float* right, int numSamples) {
    auto& bufL = delayBuffer[0];
    auto& bufR = delayBuffer[1];
    const auto bufferSize = bufL.size();

    for (int i = 0; i < numSamples; ++i) {
      float mod = std::sin(modPhase * juce::MathConstants<float>::twoPi) * (modDepth / 100.0f);
      float delaySamples = (delayTimeSeconds + mod) * sampleRate;
      delaySamples =
          std::clamp(delaySamples, 1.0f, static_cast<float>(bufferSize - 2));  // avoid overflow

      // Calculate the fractional read index in order to interpolate
      // between two samples in the delay buffer for smoother repeats...
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
        // Ping-pong delay - alternates between R/L channels
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
          // samplesUntilNextFlip = delaySamples;
          samplesUntilNextFlip = static_cast<size_t>(delaySamples);
        }

      } else {
        // Normal stereo delay
        left[i] = dryLevel * inL + wetLevel * delayedL * fadeFactor;
        right[i] = dryLevel * inR + wetLevel * delayedR * fadeFactor;

        bufL[writeIndex] = inL + delayedL * feedback;
        bufR[writeIndex] = inR + delayedR * feedback;
      }

      writeIndex = (writeIndex + 1) % bufferSize;
      modPhase = std::fmod(modPhase + modRate / sampleRate, 1.0f);
    }
  }

private:
  double sampleRate;
  const double maxDelayTime;
  float delayTimeSeconds;

  float feedback;
  float wetLevel;
  float dryLevel;

  bool pingPongFlip = false;
  size_t samplesUntilNextFlip = 1;

  float modDepth;
  float modRate;
  float modPhase;

  float fadeInAmount = 0.0f;
  float fadeInIncrement = 0.0f;

  DelayMode mode;

  std::vector<float> delayBuffer[2];  // stereo
  size_t writeIndex;
};

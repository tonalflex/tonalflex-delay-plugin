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

  Delay() : sampleRate(44100.0), maxDelayTime(2.0), writeIndex(0), modPhase(0.0f) {
    setSampleRate(sampleRate);
  }

  void setSampleRate(double newSampleRate) {
    sampleRate = newSampleRate;
    size_t bufferSize = static_cast<size_t>(sampleRate * maxDelayTime) + 1;

    for (auto& buf : delayBuffer)
      buf.assign(bufferSize, 0.0f);
  }

  void setParameters(const Parameters& params) {
    feedback = std::clamp(params.feedback, 0.0f, 0.99f);
    wetLevel = params.wetLevel;
    dryLevel = params.dryLevel;
    modDepth = params.modulationDepthSeconds;
    modRate = params.modulationRateHz;
    mode = params.mode;

    // Tempo sync
    if (params.syncToTempo)
      delayTimeSeconds = (60.0f / params.hostBpm) * params.noteDivision;
    else
      delayTimeSeconds = params.delayTimeSeconds;

    // Calculate number of samples in one repeat on delay time change
    if (std::abs(delayTimeSeconds - params.delayTimeSeconds) > 0.0001f) {
      delayTimeSeconds = params.delayTimeSeconds;
      samplesUntilNextFlip = static_cast<size_t>(delayTimeSeconds * sampleRate);
    } else {
      delayTimeSeconds = params.delayTimeSeconds;
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

      size_t delaySamples = static_cast<size_t>((delayTimeSeconds + mod) * sampleRate);
      delaySamples = std::max<size_t>(1, delaySamples);
      if (delaySamples >= bufferSize)
        continue;

      size_t readIndex = (writeIndex + bufferSize - delaySamples) % bufferSize;

      float inL = left[i];
      float inR = right[i];
      float delayedL = bufL[readIndex];
      float delayedR = bufR[readIndex];

      if (mode == DelayMode::PingPong) {
        // Ping-pong delay - alternates between R/L channels
        left[i] = dryLevel * inL + wetLevel * delayedL;
        right[i] = dryLevel * inR + wetLevel * delayedR;

        bufL[writeIndex] = inL;   // Inject input to start the ping-pong chain
        bufR[writeIndex] = 0.0f;  // Clear the second channel

        // Cross-feed feedback
        if (pingPongFlip) {
          bufR[writeIndex] += delayedL * feedback;
        } else {
          bufL[writeIndex] += delayedR * feedback;
        }

        // Flip once per full repeat/delay time
        if (--samplesUntilNextFlip <= 0) {
          pingPongFlip = !pingPongFlip;
          samplesUntilNextFlip = delaySamples;
        }

      } else {
        // Normal stereo delay
        left[i] = dryLevel * inL + wetLevel * delayedL;
        right[i] = dryLevel * inR + wetLevel * delayedR;

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

  DelayMode mode;

  std::vector<float> delayBuffer[2];  // stereo
  size_t writeIndex;
};

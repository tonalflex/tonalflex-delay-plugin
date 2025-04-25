#pragma once

#include <vector>
#include <cmath>
#include <juce_audio_processors/juce_audio_processors.h>

class StereoDelay {
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

  StereoDelay() : sampleRate(44100.0), maxDelayTime(2.0), writeIndex(0), modPhase(0.0f) {
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
    modulationDepth = params.modulationDepthSeconds;
    modulationRate = params.modulationRateHz;
    mode = params.mode;

    // Tempo sync
    if (params.syncToTempo)
      delayTimeSeconds = (60.0f / params.hostBpm) * (1.0f / params.noteDivision);
    else
      delayTimeSeconds = params.delayTimeSeconds;
  }

  void processMono(float* samples, int numSamples) {
    auto& buf = delayBuffer[0];

    for (int i = 0; i < numSamples; ++i) {
      float mod = std::sin(modPhase * juce::MathConstants<float>::twoPi) * (modulationDepth / 100);

      size_t delaySamples = static_cast<size_t>((delayTimeSeconds + mod) * sampleRate);

      size_t readIndex = (writeIndex + buf.size() - delaySamples) % buf.size();
      float delayed = buf[readIndex];
      float input = samples[i];
      float wet = delayed;
      float output = dryLevel * input + wetLevel * wet;

      samples[i] = output;
      buf[writeIndex] = input + delayed * feedback;

      writeIndex = (writeIndex + 1) % buf.size();
      modPhase = std::fmod(modPhase + modulationRate / sampleRate, 1.0f);
    }
  }

  void processStereo(float* left, float* right, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
      float mod = std::sin(modPhase * juce::MathConstants<float>::twoPi) * (modulationDepth / 100);
      size_t delaySamples = static_cast<size_t>((delayTimeSeconds + mod) * sampleRate);

      for (int ch = 0; ch < 2; ++ch) {
        auto& buf = delayBuffer[ch];
        float in = (ch == 0 ? left[i] : right[i]);

        size_t readIndex = (writeIndex + buf.size() - delaySamples) % buf.size();
        float delayed = buf[readIndex];

        float wet = delayed;
        float output = dryLevel * in + wetLevel * wet;

        if (ch == 0)
          left[i] = (mode == DelayMode::PingPong)
                        ? dryLevel * in + wetLevel * delayBuffer[1][readIndex]
                        : output;
        else
          right[i] = (mode == DelayMode::PingPong)
                         ? dryLevel * in + wetLevel * delayBuffer[0][readIndex]
                         : output;

        buf[writeIndex] = in + delayed * feedback;
      }

      writeIndex = (writeIndex + 1) % delayBuffer[0].size();
      modPhase = std::fmod(modPhase + modulationRate / sampleRate, 1.0f);
    }
  }

private:
  double sampleRate;
  const double maxDelayTime;
  float delayTimeSeconds;

  float feedback;
  float wetLevel;
  float dryLevel;

  float modulationDepth;
  float modulationRate;
  float modPhase;

  DelayMode mode;

  std::vector<float> delayBuffer[2];  // stereo
  size_t writeIndex;
};

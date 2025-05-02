#pragma once

#include <vector>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
class Delay {
public:
  enum class DelayMode { Mono, Stereo, PingPong };

  struct Parameters {
    float delayTimeSeconds = 0.5f;
    float feedback = 0.5f;
    float wetLevel = 0.5f;
    float dryLevel = 0.5f;

    float hiCutFreq = 0.0f;  // in Hz

    float modulationDepthSeconds = 0.002f;
    float modulationRateHz = 0.25f;

    bool syncToTempo = false;
    float hostBpm = 120.0f;
    float noteDivision = 0.25f;  // quarter = 0.25, dotted eighth = 0.1875 etc.

    DelayMode mode = DelayMode::Stereo;
  };

  Delay();

  void setSampleRate(double newSampleRate);
  void setParameters(const Parameters& params);

  void processMono(float* samples, int numSamples);
  void processStereo(float* left, float* right, int numSamples);

private:
  double sampleRate;
  const double maxDelayTime;

  float delayTimeSeconds;
  float feedback;
  float wetLevel;
  float dryLevel;

  float hiCutFreq;
  juce::dsp::IIR::Filter<float> hiCutFilterL;
  juce::dsp::IIR::Filter<float> hiCutFilterR;
  juce::dsp::IIR::Coefficients<float>::Ptr hiCutCoefficients;

  float modDepth;
  float modRate;
  float modPhase;

  float fadeInAmount = 0.0f;
  float fadeInIncrement = 0.0f;

  bool pingPongFlip = false;
  size_t samplesUntilNextFlip = 1;

  DelayMode mode;

  std::vector<float> delayBuffer[2];
  size_t writeIndex;
};

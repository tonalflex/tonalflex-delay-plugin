#include "plugin_processor.h"

#if !HEADLESS
#include "plugin_editor.h"
#endif

namespace audio_plugin {
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
                         ),
      parameters(*this, nullptr, "PARAMETERS", [] {
        using namespace juce;
        std::vector<std::unique_ptr<RangedAudioParameter>> params;

        params.push_back(
            std::make_unique<AudioParameterFloat>("delayTime", "Delay Time", 0.01f, 2.0f, 0.3f));
        params.push_back(
            std::make_unique<AudioParameterFloat>("feedback", "Feedback", 0.0f, 0.99f, 0.2f));
        params.push_back(std::make_unique<AudioParameterFloat>("wet", "Wet", 0.0f, 1.0f, 0.10f));
        params.push_back(std::make_unique<AudioParameterFloat>("dry", "Dry", 0.0f, 1.0f, 0.5f));
        params.push_back(
            std::make_unique<AudioParameterFloat>("modDepth", "Mod Depth", 0.0f, 0.5f, 0.2f));
        params.push_back(
            std::make_unique<AudioParameterFloat>("modRate", "Mod Rate", 0.01f, 5.0f, 1.2f));
        params.push_back(std::make_unique<AudioParameterBool>("sync", "Tempo Sync", false));
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            "division", "Note Division",
            juce::StringArray{"1/1", "1/2", "1/4", "1/8", "1/8 Dotted", "1/16"}, 2));
        params.push_back(std::make_unique<AudioParameterChoice>(
            "mode", "Delay Mode", StringArray{"Mono", "Stereo", "PingPong"}, 1));

        return juce::AudioProcessorValueTreeState::ParameterLayout{params.begin(), params.end()};
      }()) {
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {
  releaseResources();
  juce::Logger::writeToLog("[Processor] Destructor called");
}

const juce::String AudioPluginAudioProcessor::getName() const {
  return "Tonalflex Delay";
}

bool AudioPluginAudioProcessor::acceptsMidi() const {
  return false;
}
bool AudioPluginAudioProcessor::producesMidi() const {
  return false;
}
bool AudioPluginAudioProcessor::isMidiEffect() const {
  return false;
}
double AudioPluginAudioProcessor::getTailLengthSeconds() const {
  return 2.0;
}

int AudioPluginAudioProcessor::getNumPrograms() {
  return 1;
}
int AudioPluginAudioProcessor::getCurrentProgram() {
  return 0;
}
void AudioPluginAudioProcessor::setCurrentProgram(int index) {
  juce::ignoreUnused(index);
}
const juce::String AudioPluginAudioProcessor::getProgramName(int index) {
  juce::ignoreUnused(index);
  return {};
}
void AudioPluginAudioProcessor::changeProgramName(int index, const juce::String& newName) {
  juce::ignoreUnused(index, newName);
}

void AudioPluginAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
  juce::ignoreUnused(samplesPerBlock);

  delay.setSampleRate(sampleRate);

  // Ignore build warnings for unused variables
  juce::ignoreUnused(sampleRate, samplesPerBlock);
}

void AudioPluginAudioProcessor::releaseResources() {
  juce::Logger::writeToLog("[Processor] releaseResources() called");
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
      layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
    return false;

#if !JucePlugin_IsSynth
  if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
    return false;
#endif

  return true;
}

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages) {
  juce::ignoreUnused(midiMessages);
  juce::ScopedNoDenormals noDenormals;

  StereoDelay::Parameters p;
  p.delayTimeSeconds = *parameters.getRawParameterValue("delayTime");
  p.feedback = *parameters.getRawParameterValue("feedback");
  p.wetLevel = *parameters.getRawParameterValue("wet");
  p.dryLevel = *parameters.getRawParameterValue("dry");
  p.modulationDepthSeconds = *parameters.getRawParameterValue("modDepth");
  p.modulationRateHz = *parameters.getRawParameterValue("modRate");
  p.syncToTempo = *parameters.getRawParameterValue("sync") > 0.5f;

  static const std::array<float, 6> noteDurations = {1.0f, 0.5f, 0.25f, 0.125f, 0.1875f, 0.0625f};
  int divisionIndex = static_cast<int>(*parameters.getRawParameterValue("division"));
  p.noteDivision = noteDurations[divisionIndex];

  p.hostBpm = getPlayHead()->getPosition()->getBpm().orFallback(120.0);
  p.mode = static_cast<StereoDelay::DelayMode>(
      static_cast<int>(*parameters.getRawParameterValue("mode")));

  delay.setParameters(p);

  auto* left = buffer.getWritePointer(0);
  auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

  if (buffer.getNumChannels() == 1 || right == nullptr) {
    delay.processMono(left, buffer.getNumSamples());
  } else {
    delay.processStereo(left, right, buffer.getNumSamples());
  }
}

bool AudioPluginAudioProcessor::hasEditor() const {
  return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor() {
#if !HEADLESS
  return new AudioPluginAudioProcessorEditor(*this);
#else
  return nullptr;
#endif
}

void AudioPluginAudioProcessor::getStateInformation(juce::MemoryBlock& destData) {
  juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data, int sizeInBytes) {
  juce::ignoreUnused(data, sizeInBytes);
}
}  // namespace audio_plugin

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new audio_plugin::AudioPluginAudioProcessor();
}

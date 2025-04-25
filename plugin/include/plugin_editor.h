#pragma once

#include "plugin_processor.h"
#include <juce_gui_extra/juce_gui_extra.h>  // For WebBrowserComponent (used for WebView)
#include "BinaryData.h"                     // Contains embedded Web UI assets (HTML/CSS/JS)

// Restricts WebView loading to internal resources only
struct SinglePageBrowser : juce::WebBrowserComponent {
  using WebBrowserComponent::WebBrowserComponent;
  bool pageAboutToLoad(const juce::String& newURL) override {
    return newURL == juce::String("http://localhost:5173/") || newURL == getResourceProviderRoot();
  }
};

namespace audio_plugin {
class AudioPluginAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  explicit AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor&);
  ~AudioPluginAudioProcessorEditor() override;

  void paint(juce::Graphics&) override;
  void resized() override;

  int getControlParameterIndex(juce::Component&) override {
    return controlParameterIndexReceiver.getControlParameterIndex();
  }

private:
  AudioPluginAudioProcessor& processorRef;

  //==============================================================================
  // WebView UI
  //==============================================================================
  std::unique_ptr<juce::WebBrowserComponent> webView;

  juce::WebControlParameterIndexReceiver controlParameterIndexReceiver;

  juce::WebSliderRelay delayTimeRelay{"delayTime"};
  juce::WebSliderRelay feedbackRelay{"feedback"};
  juce::WebSliderRelay wetRelay{"wet"};
  juce::WebSliderRelay dryRelay{"dry"};

  juce::WebSliderParameterAttachment delayTimeWebAttachment{
      *processorRef.parameters.getParameter("delayTime"), delayTimeRelay, nullptr};
  juce::WebSliderParameterAttachment feedbackWebAttachment{
      *processorRef.parameters.getParameter("feedback"), feedbackRelay, nullptr};
  juce::WebSliderParameterAttachment wetWebAttachment{*processorRef.parameters.getParameter("wet"),
                                                      wetRelay, nullptr};
  juce::WebSliderParameterAttachment dryWebAttachment{*processorRef.parameters.getParameter("dry"),
                                                      dryRelay, nullptr};

  std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);
  juce::String getMimeForExtension(const juce::String& extension);

  //==============================================================================
  // Native JUCE UI
  //==============================================================================
  // Delay parameters
  juce::Label delayTimeLabel, feedbackLabel, wetLabel, dryLabel;
  juce::Slider delayTimeSlider, feedbackSlider, wetSlider, drySlider;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayTimeAttachment,
      feedbackAttachment, wetAttachment, dryAttachment;

  // Trail modulation
  juce::Label modDepthLabel, modRateLabel;
  juce::Slider modDepthSlider, modRateSlider;
  std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> modDepthAttachment,
      modRateAttachment;

  // Sync toggle
  juce::ToggleButton syncToggle;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncAttachment;

  // Tempo sync division
  // (1/1, 1/2, 1/4, 1/8, 1/8 Dotted, 1/16)
  juce::Label divisionLabel;
  juce::ComboBox divisionBox;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> divisionAttachment;

  // Mode selector (Mono, Stereo, PingPong)
  juce::Label modeLabel;
  juce::ComboBox modeBox;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> modeAttachment;

  //==============================================================================

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessorEditor)
};
}  // namespace audio_plugin

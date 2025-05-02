#include "plugin_editor.h"
#include "plugin_processor.h"

namespace audio_plugin {
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p) {
  /**
   * Initialize WebView UI
   *
   * - The WebView UI is built using a web-framework (see the `/ui` directory in the project root).
   * - The compiled HTML/CSS/JS assets are embedded into the plugin via JUCE's BinaryData system.
   * - This Web UI is rendered inside the plugin editor using JUCE's WebBrowserComponent.
   * - Communication between the C++ backend and the WebView is handled via native integration.
   */
  webView = std::make_unique<juce::WebBrowserComponent>(
      juce::WebBrowserComponent::Options{}
          .withNativeIntegrationEnabled()  // (C++ <=> JS bridge, events, etc.)

          // Explicitly use WebView2 backend on Windows for modern HTML/CSS/JS support
          // JUCE defaults to WebKit on macOS/Linux
          .withBackend(juce::WebBrowserComponent::Options::Backend::webview2)
          .withWinWebView2Options(
              juce::WebBrowserComponent::Options::WinWebView2{}.withUserDataFolder(
                  juce::File::getSpecialLocation(juce::File::tempDirectory)))

          // Provide WebView UI resources from JUCE BinaryData (HTML/CSS/JS, etc.)
          .withResourceProvider([this](const auto& url) { return getResource(url); },
                                juce::URL{"http://localhost:5173/"}.getOrigin())

          // Add support for control focus tracking in the WebView (parameter automation)
          .withOptionsFrom(controlParameterIndexReceiver)

          // Bind parameter relays for two-way communication (C++ <=> JS)
          .withOptionsFrom(hiCutFreqRelay)
          .withOptionsFrom(delayTimeRelay)
          .withOptionsFrom(feedbackRelay)
          .withOptionsFrom(wetRelay)
          .withOptionsFrom(dryRelay)
          .withOptionsFrom(modDepthRelay)
          .withOptionsFrom(modRateRelay)
          .withOptionsFrom(sync)
          .withOptionsFrom(divisionRelay)
          .withOptionsFrom(modeRelay)

          // Example: register a JUCE C++ function callable from JS for debugging/testing
          .withNativeFunction(
              "exampleNativeFunction",
              [](const juce::Array<juce::var>& args,
                 juce::WebBrowserComponent::NativeFunctionCompletion completion) {
                juce::Logger::writeToLog("exampleNativeFunction called from WebView");
                for (int i = 0; i < args.size(); ++i)
                  juce::Logger::writeToLog("Arg " + juce::String(i) + ": " + args[i].toString());
                completion("Hello from JUCE native function!");
              })

          // Inject debug message into browser console on load
          .withUserScript(R"(console.log("JUCE C++ Backend is running!");)"));

  // Ensure WebView is added after full construction (avoids timing issues)
  juce::MessageManager::callAsync([this]() {
    addAndMakeVisible(*webView);
    webView->goToURL(juce::WebBrowserComponent::getResourceProviderRoot() + "index.html");
    // webView->goToURL("http://localhost:5173/");
  });

  // Set size of desktop plugin window (pixels)
  setSize(430, 830);

  /**
   * Initialize Native JUCE UI (for development and demo purposes)
   */
  // addAndMakeVisible(hiCutFreqLabel);
  // hiCutFreqLabel.setText("Hi Cut Freq", juce::dontSendNotification);
  // hiCutFreqLabel.setJustificationType(juce::Justification::centred);
  // addAndMakeVisible(hiCutFreqSlider);
  // hiCutFreqSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  // hiCutFreqSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  // hiCutFreqSlider.setRange(2000.0f, 20000.0f, 100.0f);
  // hiCutFreqSlider.setValue(20000.0f);
  // hiCutFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
  //     processorRef.getParameters(), "hiCutFreq", hiCutFreqSlider);

  // addAndMakeVisible(delayTimeLabel);
  // delayTimeLabel.setText("Delay Time", juce::dontSendNotification);
  // delayTimeLabel.setJustificationType(juce::Justification::centred);
  // addAndMakeVisible(delayTimeSlider);
  // delayTimeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  // delayTimeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  // delayTimeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
  //     processorRef.getParameters(), "delayTime", delayTimeSlider);

  // addAndMakeVisible(feedbackLabel);
  // feedbackLabel.setText("Feedback", juce::dontSendNotification);
  // feedbackLabel.setJustificationType(juce::Justification::centred);
  // addAndMakeVisible(feedbackSlider);
  // feedbackSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  // feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  // feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
  //     processorRef.getParameters(), "feedback", feedbackSlider);

  // addAndMakeVisible(wetLabel);
  // wetLabel.setText("Wet Level", juce::dontSendNotification);
  // wetLabel.setJustificationType(juce::Justification::centred);
  // addAndMakeVisible(wetSlider);
  // wetSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  // wetSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  // wetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
  //     processorRef.getParameters(), "wet", wetSlider);

  // addAndMakeVisible(dryLabel);
  // dryLabel.setText("Dry Level", juce::dontSendNotification);
  // dryLabel.setJustificationType(juce::Justification::centred);
  // addAndMakeVisible(drySlider);
  // drySlider.setSliderStyle(juce::Slider::LinearHorizontal);
  // drySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  // dryAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
  //     processorRef.getParameters(), "dry", drySlider);

  // addAndMakeVisible(modDepthLabel);
  // modDepthLabel.setText("Mod Depth", juce::dontSendNotification);
  // modDepthLabel.setJustificationType(juce::Justification::centred);
  // addAndMakeVisible(modDepthSlider);
  // modDepthSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  // modDepthSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  // modDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
  //     processorRef.getParameters(), "modDepth", modDepthSlider);

  // addAndMakeVisible(modRateLabel);
  // modRateLabel.setText("Mod Rate", juce::dontSendNotification);
  // modRateLabel.setJustificationType(juce::Justification::centred);
  // addAndMakeVisible(modRateSlider);
  // modRateSlider.setSliderStyle(juce::Slider::LinearHorizontal);
  // modRateSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
  // modRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
  //     processorRef.getParameters(), "modRate", modRateSlider);

  // addAndMakeVisible(syncToggle);
  // syncToggle.setButtonText("Sync to Tempo");
  // syncAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
  //     processorRef.getParameters(), "sync", syncToggle);

  // addAndMakeVisible(divisionLabel);
  // divisionLabel.setText("Note Division", juce::dontSendNotification);
  // divisionLabel.setJustificationType(juce::Justification::centred);
  // addAndMakeVisible(divisionBox);
  // divisionBox.addItem("1/1", 1);         // 1.0
  // divisionBox.addItem("1/2", 2);         // 0.5
  // divisionBox.addItem("1/4", 3);         // 0.25
  // divisionBox.addItem("1/8", 4);         // 0.125
  // divisionBox.addItem("1/8 Dotted", 5);  // 0.1875
  // divisionBox.addItem("1/16", 6);        // 0.0625
  // divisionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
  //     processorRef.getParameters(), "division", divisionBox);

  // addAndMakeVisible(modeLabel);
  // modeLabel.setText("Delay Mode", juce::dontSendNotification);
  // modeLabel.setJustificationType(juce::Justification::centred);
  // addAndMakeVisible(modeBox);
  // modeBox.addItem("Mono", 1);
  // modeBox.addItem("Stereo", 2);
  // modeBox.addItem("PingPong", 3);
  // modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
  //     processorRef.getParameters(), "mode", modeBox);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
  webView->setVisible(false);
  webView->stop();
  juce::Logger::writeToLog("~AudioPluginAudioProcessorEditor destroyed");
}

void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
  // g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  // g.setColour(juce::Colours::white);
  // g.setFont(15.0f);
}

void AudioPluginAudioProcessorEditor::resized() {
  auto bounds = getLocalBounds();

  // auto leftPanel = bounds.removeFromLeft(bounds.getWidth() / 2);

  // auto sliderHeight = 50;

  // hiCutFreqLabel.setBounds(leftPanel.removeFromTop(20));
  // hiCutFreqSlider.setBounds(leftPanel.removeFromTop(sliderHeight));

  // delayTimeLabel.setBounds(leftPanel.removeFromTop(20));
  // delayTimeSlider.setBounds(leftPanel.removeFromTop(sliderHeight));

  // feedbackLabel.setBounds(leftPanel.removeFromTop(20));
  // feedbackSlider.setBounds(leftPanel.removeFromTop(sliderHeight));

  // wetLabel.setBounds(leftPanel.removeFromTop(20));
  // wetSlider.setBounds(leftPanel.removeFromTop(sliderHeight));

  // dryLabel.setBounds(leftPanel.removeFromTop(20));
  // drySlider.setBounds(leftPanel.removeFromTop(sliderHeight));

  // modDepthLabel.setBounds(leftPanel.removeFromTop(20));
  // modDepthSlider.setBounds(leftPanel.removeFromTop(sliderHeight));

  // modRateLabel.setBounds(leftPanel.removeFromTop(20));
  // modRateSlider.setBounds(leftPanel.removeFromTop(sliderHeight));

  // syncToggle.setBounds(leftPanel.removeFromTop(30));

  // divisionLabel.setBounds(leftPanel.removeFromTop(20));
  // divisionBox.setBounds(leftPanel.removeFromTop(30));

  // modeLabel.setBounds(leftPanel.removeFromTop(20));
  // modeBox.setBounds(leftPanel.removeFromTop(30));

  if (webView)
    webView->setBounds(bounds);  // Set web view bounds to the right half
}

// Get the WebView UI resources from BinaryData
std::optional<juce::WebBrowserComponent::Resource> AudioPluginAudioProcessorEditor::getResource(
    const juce::String& url) {
  juce::Logger::writeToLog("Requested URL: " + url);

  // Extract filename and normalize to match BinaryData naming
  juce::String filename = juce::URL(url).getFileName().trim();
  juce::String resourceName = filename.removeCharacters("-").replaceCharacter('.', '_');

  int size = 0;
  const char* data = BinaryData::getNamedResource(resourceName.toRawUTF8(), size);

  if (data == nullptr || size <= 0) {
    juce::Logger::writeToLog("Resource not found or empty: " + resourceName);
    return std::nullopt;
  }

  std::vector<std::byte> content(static_cast<size_t>(size));
  std::memcpy(content.data(), data, static_cast<size_t>(size));

  juce::String ext = filename.fromLastOccurrenceOf(".", false, false).toLowerCase();
  juce::String mime = getMimeForExtension(ext);
  if (mime.isEmpty())
    mime = "application/octet-stream";

  juce::Logger::writeToLog("Returning resource: " + resourceName + " (" + mime + ")");
  return juce::WebBrowserComponent::Resource{std::move(content), mime};
}

// Map file extensions to MIME types for serving embedded resources in the WebView UI
juce::String AudioPluginAudioProcessorEditor::getMimeForExtension(const juce::String& extension) {
  static const std::unordered_map<juce::String, juce::String> mimeMap = {
      {"htm", "text/html"},
      {"html", "text/html"},
      {"txt", "text/plain"},
      {"jpg", "image/jpeg"},
      {"jpeg", "image/jpeg"},
      {"svg", "image/svg+xml"},
      {"ico", "image/vnd.microsoft.icon"},
      {"json", "application/json"},
      {"png", "image/png"},
      {"css", "text/css"},
      {"map", "application/json"},
      {"js", "text/javascript"},
      {"woff2", "font/woff2"}};

  const auto lower = extension.toLowerCase();

  if (const auto it = mimeMap.find(lower); it != mimeMap.end())
    return it->second;

  jassertfalse;
  return "application/octet-stream";
}

}  // namespace audio_plugin

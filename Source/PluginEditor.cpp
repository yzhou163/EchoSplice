#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

VERBINPEABODYAudioProcessorEditor::VERBINPEABODYAudioProcessorEditor (VERBINPEABODYAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    startTimerHz(10);
    setSize (1005, 600);
    backgroundImage = juce::ImageFileFormat::loadFrom (BinaryData::Frame_png,
                                                         BinaryData::Frame_pngSize);
    sliderHeadImage = juce::ImageFileFormat::loadFrom(BinaryData::sliderhead_png, BinaryData::sliderhead_pngSize);
    largeKnobImage = juce::ImageFileFormat::loadFrom(BinaryData::largeKnob_png, BinaryData::largeKnob_pngSize);
    middleKnobImage = juce::ImageFileFormat::loadFrom(BinaryData::middleKnob_png, BinaryData::middleKnob_pngSize);
    smallKnobImage = juce::ImageFileFormat::loadFrom(BinaryData::smallKnob_png, BinaryData::smallKnob_pngSize);
    
    HallWay = juce::ImageCache::getFromMemory(BinaryData::HallWay_jpeg, BinaryData::HallWay_jpegSize);
    MediumStairWay = juce::ImageCache::getFromMemory(BinaryData::MediumStairWay_jpeg, BinaryData::MediumStairWay_jpegSize);
    FriedbergStair = juce::ImageCache::getFromMemory(BinaryData::FriedbergStair_jpeg, BinaryData::FriedbergStair_jpegSize);
    Bathroom = juce::ImageCache::getFromMemory(BinaryData::Bathroom_jpeg, BinaryData::Bathroom_jpegSize);
    PeabodyStair = juce::ImageCache::getFromMemory(BinaryData::PeabodyStair_jpeg, BinaryData::PeabodyStair_jpegSize);
    FriedbergClose = juce::ImageCache::getFromMemory(BinaryData::FriedbergClose_jpeg, BinaryData::FriedbergClose_jpegSize);
    FriedbergAudience = juce::ImageCache::getFromMemory(BinaryData::FriedbergAudience_jpeg, BinaryData::FriedbergAudience_jpegSize);
    
    inputSliderLAF   = std::make_unique<VerticalSliderLookAndFeel>(sliderHeadImage);
    outputSliderLAF  = std::make_unique<VerticalSliderLookAndFeel>(sliderHeadImage);
    reverbSliderLAF  = std::make_unique<VerticalSliderLookAndFeel>(sliderHeadImage);
    preDelayLAF      = std::make_unique<RotarySliderLookAndFeel>(largeKnobImage);
    brightnessLAF    = std::make_unique<RotarySliderLookAndFeel>(largeKnobImage);
    granularMixLAF        = std::make_unique<RotarySliderLookAndFeel>(middleKnobImage);
    grainSizeLAF          = std::make_unique<RotarySliderLookAndFeel>(middleKnobImage);
    granularFeedbackLAF   = std::make_unique<RotarySliderLookAndFeel>(middleKnobImage);
    tailStart1LAF = std::make_unique<RotarySliderLookAndFeel> (middleKnobImage);
    tailStart2LAF = std::make_unique<RotarySliderLookAndFeel>(middleKnobImage);
    tailEnd1LAF =   std::make_unique<RotarySliderLookAndFeel> (middleKnobImage);
    tailEnd2LAF = std::make_unique<RotarySliderLookAndFeel> (middleKnobImage);
    convMixLAF = std::make_unique<HorizontalSliderLookAndFeel> (smallKnobImage);

    

    // set LAFs on sliders
    inputGainSlider.setLookAndFeel(inputSliderLAF.get());
    outputGainSlider.setLookAndFeel(outputSliderLAF.get());
    reverbDryWetSlider.setLookAndFeel(reverbSliderLAF.get());
    preDelaySlider.setLookAndFeel(preDelayLAF.get());
    brightnessSlider.setLookAndFeel(brightnessLAF.get());
    granularMixSlider.setLookAndFeel(granularMixLAF.get());
    grainSizeSlider.setLookAndFeel(grainSizeLAF.get());
    granularFeedbackSlider.setLookAndFeel(granularFeedbackLAF.get());
    tailStart1Slider.setLookAndFeel(tailStart1LAF.get());
    tailStart2Slider.setLookAndFeel(tailStart2LAF.get());
    tailEnd1Slider.setLookAndFeel(tailEnd1LAF.get());
    tailEnd2Slider.setLookAndFeel(tailEnd2LAF.get());
    convMixSlider.setLookAndFeel(convMixLAF.get());
    
    audioProcessor.ir1ChangeBroadcaster.addChangeListener(this);
    audioProcessor.ir2ChangeBroadcaster.addChangeListener(this);

    // IR length labels
    ir1LengthLabel.setText("IR1: -- s ( -- samples )", juce::dontSendNotification);
    ir1LengthLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(ir1LengthLabel);

    ir2LengthLabel.setText("IR2: -- s ( -- samples )", juce::dontSendNotification);
    ir2LengthLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(ir2LengthLabel);

    // BPM & sync controls
    addAndMakeVisible(bpmLabel);
    bpmLabel.setText("BPM: --", juce::dontSendNotification);
    bpmLabel.setJustificationType(juce::Justification::centred);

    addAndMakeVisible(syncToBPMButton);
 //   syncToBPMButton.setButtonText("Sync to BPM");
    syncToBPMAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts, "syncToBPM", syncToBPMButton);

    addAndMakeVisible(syncLabel);
    syncLabel.setText("Sync", juce::dontSendNotification);
    syncLabel.setJustificationType(juce::Justification::centred);

    // Input & Output sliders
    inputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    inputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    addAndMakeVisible(inputGainSlider);
    inputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "inputGain", inputGainSlider);

    outputGainSlider.setSliderStyle(juce::Slider::LinearVertical);
    outputGainSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    addAndMakeVisible(outputGainSlider);
    outputGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "outputGain", outputGainSlider);

    // PreDelay slider
    preDelaySlider.setSliderStyle(juce::Slider::Rotary);
    preDelaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 60, 20);
    addAndMakeVisible(preDelaySlider);
    preDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "preDelay", preDelaySlider);

    // IR ComboBoxes
    auto addIRItems = [](juce::ComboBox& box)
    {
        box.addItem("Hallway 1", 1);
        box.addItem("Hallway 2", 2);
        box.addItem("MediumStairWay 1", 3);
        box.addItem("MediumStairWay 2", 4);
        box.addItem("MediumStairWay 3", 5);
        box.addItem("MediumStairWay 4", 6);
        box.addItem("MediumStairWay5", 7);
        box.addItem("Bathroom 1", 8);
        box.addItem("Bathroom 2", 9);
        box.addItem("Bathroom 3", 10);
        box.addItem("Bathroom 4", 11);
        box.addItem("Peabody Stair 1", 12);
        box.addItem("Peabody Stair 2", 13);
        box.addItem("Peabody Stair 3", 14);
        box.addItem("Peabody Stair 4", 15);
        box.addItem("Friedberg Stair 1", 16);
        box.addItem("Friedberg Stair 2", 17);
        box.addItem("Friedberg Stair 3", 18);
        box.addItem("Friedberg Stair 4", 19);
        box.addItem("Friedberg Hall Close1", 20);
        box.addItem("Friedberg Hall Close2", 21);
        box.addItem("Friedberg Hall Close3", 22);
        box.addItem("Friedberg Hall Audience1", 23);
        box.addItem("Friedberg Hall Audience2", 24);
        box.addItem("Friedberg Hall Audience3", 25);
        
        
    };
    
    

    addIRItems(irComboBox);
    addIRItems(irComboBox2);

    addAndMakeVisible(irComboBox);
    addAndMakeVisible(irComboBox2);

    irComboBox.addListener(this);
    irComboBox2.addListener(this);
    
    irChoice1Attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "irChoice1", irComboBox);
    irChoice2Attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "irChoice2", irComboBox2);
    
    // bypass
    bypassButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        audioProcessor.apvts,
        "bypass",
        bypassButton
    );

   // bypassButton.setButtonText("Bypass");
    addAndMakeVisible(bypassButton);

    // Labels
 //   addAndMakeVisible(inputLabel);
    inputLabel.setText("Input", juce::dontSendNotification);
    inputLabel.setJustificationType(juce::Justification::centred);

//    addAndMakeVisible(outputLabel);
    outputLabel.setText("Output", juce::dontSendNotification);
    outputLabel.setJustificationType(juce::Justification::centred);

 //   addAndMakeVisible(preDelayLabel);
    preDelayLabel.setText("Pre Delay", juce::dontSendNotification);
    preDelayLabel.setJustificationType(juce::Justification::centred);
    
    //brightness
    addAndMakeVisible(brightnessSlider);
    brightnessSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    brightnessSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 20);

 //   addAndMakeVisible(brightness);
    brightness.setText("Brightness", juce::dontSendNotification);
    brightness.setJustificationType(juce::Justification::centred);
    
    // attach to APVTS
    brightnessAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "brightness", brightnessSlider);

    //grain
    // Granular Mix slider
    granularMixSlider.setSliderStyle(juce::Slider::Rotary);
    granularMixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    addAndMakeVisible(granularMixSlider);
    granularMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "granularMix", granularMixSlider);

   // addAndMakeVisible(granularMix);
    granularMix.setText("granularMix", juce::dontSendNotification);
    granularMix.setJustificationType(juce::Justification::centred);
    
    
    // Grain Size slider
    grainSizeSlider.setSliderStyle(juce::Slider::Rotary);
    grainSizeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    addAndMakeVisible(grainSizeSlider);
    grainSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "grainSize", grainSizeSlider);

  //  addAndMakeVisible(grainSize);
    grainSize.setText("grainSize", juce::dontSendNotification);
    grainSize.setJustificationType(juce::Justification::centred);
    
    
    // GranularFeedback slider
    granularFeedbackSlider.setSliderStyle(juce::Slider::Rotary);
    granularFeedbackSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    addAndMakeVisible(granularFeedbackSlider);
    granularFeedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "granularFeedback", granularFeedbackSlider);

 //   addAndMakeVisible(granularFeedback);
    granularFeedback.setText("granularFeedback", juce::dontSendNotification);
    granularFeedback.setJustificationType(juce::Justification::centred);
    
    
    // Convolution mix slider
    convMixSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    convMixSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 20);
    addAndMakeVisible(convMixSlider);
    convMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "convMix", convMixSlider);

    // Reverb dry/wet slider
    reverbDryWetSlider.setSliderStyle(juce::Slider::LinearVertical);
    reverbDryWetSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 50, 20);
    addAndMakeVisible(reverbDryWetSlider);
    reverbDryWetAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "reverbDryWet", reverbDryWetSlider);
    
    //Labels for convMix and reverbDryWet sliders
   // addAndMakeVisible(convMixLabel);
    convMixLabel.setText("Conv Mix", juce::dontSendNotification);
    convMixLabel.setJustificationType(juce::Justification::centred);

   // addAndMakeVisible(reverbDryWetLabel);
    reverbDryWetLabel.setText("Reverb Dry/Wet", juce::dontSendNotification);
    reverbDryWetLabel.setJustificationType(juce::Justification::centred);
    
    // --- Tail Start 1 ---
        tailStart1Slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    tailStart1Slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 60, 20);
        addAndMakeVisible(tailStart1Slider);
        tailStart1Label.setJustificationType(juce::Justification::centred);
        tailStart1Label.setText("Tail Start 1", juce::dontSendNotification);
   //     addAndMakeVisible(tailStart1Label);
    tailStart1Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "tailStart1", tailStart1Slider);
    
        // --- Tail End 1 ---
        tailEnd1Slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        tailEnd1Slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 60, 20);
        addAndMakeVisible(tailEnd1Slider);
        tailEnd1Label.setJustificationType(juce::Justification::centred);
        tailEnd1Label.setText("Tail End 1", juce::dontSendNotification);
     //   addAndMakeVisible(tailEnd1Label);
    tailEnd1Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "tailEnd1", tailEnd1Slider);
    
        // --- Tail Start 2 ---
        tailStart2Slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    tailStart2Slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 60, 20);
        addAndMakeVisible(tailStart2Slider);
        tailStart2Label.setJustificationType(juce::Justification::centred);
        tailStart2Label.setText("Tail Start 2", juce::dontSendNotification);
     //   addAndMakeVisible(tailStart2Label);
    tailStart2Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "tailStart2", tailStart2Slider);

        // --- Tail End 2 ---
        tailEnd2Slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        tailEnd2Slider.setTextBoxStyle(juce::Slider::NoTextBox, true, 60, 20);
        addAndMakeVisible(tailEnd2Slider);
        tailEnd2Label.setJustificationType(juce::Justification::centred);
        tailEnd2Label.setText("Tail End 2", juce::dontSendNotification);
     //   addAndMakeVisible(tailEnd2Label);
    tailEnd2Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "tailEnd2", tailEnd2Slider);

//    inputGainSlider.setLookAndFeel(&inputSliderLAF);
//    outputGainSlider.setLookAndFeel(&outputSliderLAF);
//    reverbDryWetSlider.setLookAndFeel(&reverbSliderLAF);
//    preDelaySlider.setLookAndFeel(&preDelayLAF);
//    brightnessSlider.setLookAndFeel(&brightnessLAF);
//    
//
    
    
    irComboBox.setColour(juce::ComboBox::backgroundColourId, juce::Colours::black);
    irComboBox.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    irComboBox.setColour(juce::ComboBox::outlineColourId, juce::Colours::black);

    irComboBox2.setColour(juce::ComboBox::backgroundColourId, juce::Colours::black);
    irComboBox2.setColour(juce::ComboBox::textColourId, juce::Colours::white);
    irComboBox2.setColour(juce::ComboBox::outlineColourId, juce::Colours::black);
}

//==============================================================================

VERBINPEABODYAudioProcessorEditor::~VERBINPEABODYAudioProcessorEditor()
{
    stopTimer();
    
    inputGainSlider.setLookAndFeel(nullptr);
      outputGainSlider.setLookAndFeel(nullptr);
      reverbDryWetSlider.setLookAndFeel(nullptr);
      preDelaySlider.setLookAndFeel(nullptr);
      brightnessSlider.setLookAndFeel(nullptr);
    granularMixSlider.setLookAndFeel(nullptr);
        grainSizeSlider.setLookAndFeel(nullptr);
        granularFeedbackSlider.setLookAndFeel(nullptr);
    tailStart1LAF = std::make_unique<RotarySliderLookAndFeel> (middleKnobImage);
    tailStart2LAF = std::make_unique<RotarySliderLookAndFeel>(middleKnobImage);
    tailEnd1LAF =   std::make_unique<RotarySliderLookAndFeel> (middleKnobImage);
    tailEnd2LAF = std::make_unique<RotarySliderLookAndFeel> (middleKnobImage);
    convMixLAF = std::make_unique<HorizontalSliderLookAndFeel> (smallKnobImage);
    
    audioProcessor.ir1ChangeBroadcaster.removeChangeListener(this);
    audioProcessor.ir2ChangeBroadcaster.removeChangeListener(this);
}

//==============================================================================

void VERBINPEABODYAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.drawImage (backgroundImage, getLocalBounds().toFloat());
  //  g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
  //  g.setColour(juce::Colours::blue);
    
    if (overlayImage1.isValid())
          g.drawImage(overlayImage1, 176, 135, 300, 200, 0, 0, overlayImage1.getWidth(), overlayImage1.getHeight());

      if (overlayImage2.isValid())
          g.drawImage(overlayImage2, 520, 135, 300, 200, 0, 0, overlayImage2.getWidth(), overlayImage2.getHeight());
    g.setFont(15.0f);
}

void VERBINPEABODYAudioProcessorEditor::resized()
{
    auto set = [this](auto& comp, float x, float y, float w, float h)
    {
        comp.setBounds (juce::roundToInt(x), juce::roundToInt(y),
                        juce::roundToInt(w), juce::roundToInt(h));
    };

    set(irComboBox, 20.0f, 77.0f, 360.0f, 20.0f);
    set(irComboBox2, 610.0f, 77.0f, 360.0f, 20.0f);
    set(ir1LengthLabel, 20.0f, 102.0f, 360.0f, 18.0f);
    set(ir2LengthLabel, 610.0f, 102.0f, 360.0f, 18.0f);
    set(inputGainSlider, 58.4f, 162.3f, 15.4f, 260.3f);
    set(reverbDryWetSlider, 893.8f, 162.3f, 15.4f, 260.3f);
    set(outputGainSlider, 955.4f, 162.3f, 15.4f, 260.3f);
    set(convMixSlider, 383.3f, 385.6f, 234.4f, 15.7f);
    set(granularMixSlider, 18.4f, 480.3f, 50.0f, 50.0f);
    set(grainSizeSlider, 127.0f, 480.3f, 50.0f, 50.0f);
    set(granularFeedbackSlider, 235.0f, 480.3f, 50.0f, 50.0f);
    set(tailStart1Slider, 385.3f, 465.0f, 35.0f, 35.0f);
    set(tailStart2Slider, 576.0f, 465.0f, 35.0f, 35.0f);
    set(tailEnd1Slider, 379.3f, 526.5f, 45.0f, 45.0f);
    set(tailEnd2Slider, 570.0f, 526.5f, 45.0f, 45.0f);
    set(brightnessSlider, 881.0f, 490.8f, 80.0f, 80.0f);
    set(preDelaySlider, 733.8f, 490.8f, 80.0f, 80.0f);

    

    syncToBPMButton.setBounds(800, 470, 80, 25);
    bypassButton.setBounds(10, 10, 80, 25);
}




//==============================================================================

void VERBINPEABODYAudioProcessorEditor::comboBoxChanged(juce::ComboBox* comboBox)
{
    if (comboBox == &irComboBox)
    {
        int selectedIndex = irComboBox.getSelectedId() - 1;
        overlayImage1 = getImageForIR(selectedIndex);
        float tailStart = (float)tailStart1Slider.getValue();
        float tailEnd   = (float)tailEnd1Slider.getValue();
        audioProcessor.ReadImpulseResponse(selectedIndex,/* bypassDecay1,*/ tailStart, tailEnd);
        
        repaint();
    }
    else if (comboBox == &irComboBox2)
    {
        int selectedIndex = irComboBox2.getSelectedId() - 1;
        overlayImage2 = getImageForIR(selectedIndex);
        float tailStart = (float)tailStart2Slider.getValue();
        float tailEnd   = (float)tailEnd2Slider.getValue();
        audioProcessor.ReadImpulseResponse2(selectedIndex, /*bypassDecay2,*/ tailStart, tailEnd);
        repaint();
    }
}

juce::Image VERBINPEABODYAudioProcessorEditor::getImageForIR(int index)
{
    switch (index)
    {
        case 0: case 1:  return HallWay;
        case 2: case 3: case 4: case 5: case 6: return MediumStairWay;
        case 7: case 8: case 9: case 10: return Bathroom;
        case 11: case 12: case 13: case 14: return PeabodyStair;
        case 15: case 16: case 17: case 18: return FriedbergStair;
        case 19: case 20: case 21: return FriedbergClose;
        case 22: case 23: case 24: return FriedbergAudience;
        default: return juce::Image(); // blank image
    }
}



void VERBINPEABODYAudioProcessorEditor::bypassButtonClicked()
{
    
}

void VERBINPEABODYAudioProcessorEditor::timerCallback()
{
    // IR lengths
    float s1 = audioProcessor.getIROriginalLengthSeconds1();
    float s2 = audioProcessor.getIROriginalLengthSeconds2();
    int samples1 = audioProcessor.getIROriginalSamples1();
    int samples2 = audioProcessor.getIROriginalSamples2();

    juce::String text1 = juce::String::formatted("IR1: %.3f s", s1, samples1);
    juce::String text2 = juce::String::formatted("IR2: %.3f s", s2, samples2);

    if (ir1LengthLabel.getText() != text1)
        ir1LengthLabel.setText(text1, juce::dontSendNotification);
    if (ir2LengthLabel.getText() != text2)
        ir2LengthLabel.setText(text2, juce::dontSendNotification);

    // BPM display
    double bpm = 0.0;
    if (auto* playHead = audioProcessor.getPlayHead()) {
        juce::AudioPlayHead::CurrentPositionInfo posInfo;
        if (playHead->getCurrentPosition(posInfo))
            bpm = posInfo.bpm;
    }
    bpmLabel.setText("BPM: " + juce::String(bpm, 3), juce::dontSendNotification);

    bool syncEnabled = audioProcessor.apvts.getRawParameterValue("syncToBPM")->load() > 0.5f;
    if (syncEnabled) {
        float bpmPredelay = audioProcessor.getTargetPredelayMs();

        preDelaySlider.setValue(bpmPredelay, juce::dontSendNotification);
    }
}

void VERBINPEABODYAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    //
    auto updateSlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& paramID)
    {
        if (auto* param = dynamic_cast<juce::AudioParameterFloat*>(audioProcessor.apvts.getParameter(paramID)))
        {
            // Update slider range to match parameter range
            slider.setRange(param->range.start, param->range.end, 0.001);

            // Clamp slider to range
            double clampedValue = juce::jlimit<double>(
                static_cast<double>(param->range.start),
                static_cast<double>(param->range.end),
                slider.getValue()
            );

   slider.setValue(clampedValue, juce::sendNotificationAsync);

           //  Update label display
        label.setText(paramID + ": " + juce::String(slider.getValue(), 3) + " s", juce::dontSendNotification);
        }
    };

    // Case 1: IR1 was changed
    if (source == &audioProcessor.ir1ChangeBroadcaster)
    {
        updateSlider(tailStart1Slider, tailStart1Label, "tailStart1");
        updateSlider(tailEnd1Slider,   tailEnd1Label,   "tailEnd1");

        // Optionally update IR1 length display
        ir1LengthLabel.setText(
            juce::String(audioProcessor.irOriginalLengthSeconds1.load(), 2) + " s",
            juce::dontSendNotification
        );
    }
    //  Case 2: IR2 was changed
    else if (source == &audioProcessor.ir2ChangeBroadcaster)
    {
        updateSlider(tailStart2Slider, tailStart2Label, "tailStart2");
        updateSlider(tailEnd2Slider,   tailEnd2Label,   "tailEnd2");

        // Optionally update IR2 length display
        ir2LengthLabel.setText(
            juce::String(audioProcessor.irOriginalLengthSeconds2.load(), 2) + " s",
            juce::dontSendNotification
        );
    }
}







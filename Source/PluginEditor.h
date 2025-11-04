/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================

class VERBINPEABODYAudioProcessorEditor  : public juce::AudioProcessorEditor,
                                           public juce::ComboBox::Listener,
                                           public juce::ChangeListener,
                                           private juce::Timer
{
public:
    VERBINPEABODYAudioProcessorEditor (VERBINPEABODYAudioProcessor&);
    ~VERBINPEABODYAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void comboBoxChanged(juce::ComboBox* comboBox) override;
    void bypassButtonClicked();
    void timerCallback() override;
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
    // Images
    juce::Image backgroundImage;
    juce::Image sliderHeadImage;
    juce::Image largeKnobImage;
    juce::Image middleKnobImage;
    juce::Image smallKnobImage;
    
    juce::Image HallWay;
    juce::Image MediumStairWay;
    juce::Image FriedbergStair;
    juce::Image Bathroom;
    juce::Image PeabodyStair;
    juce::Image FriedbergClose;
    juce::Image FriedbergAudience;
    juce::Image overlayImage1;
    juce::Image overlayImage2;
    juce::Image getImageForIR(int index);
    
    
    

    VERBINPEABODYAudioProcessor& audioProcessor;

    class VerticalSliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        VerticalSliderLookAndFeel(juce::Image& img) : sliderImage(img) {}
        void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle style, juce::Slider& slider) override
        {
            if (!sliderImage.isValid())
                return;

            float widthScale  = 0.5f; // scale width if you want
            float heightScale = 0.2f; // half the height

            int imgWidth  = static_cast<int>(sliderImage.getWidth() * widthScale);
            int imgHeight = static_cast<int>(sliderImage.getHeight() * heightScale);

            float cx = x + (width - imgWidth) * 0.5f;
            float cy = juce::jlimit<float>(y, y + height - imgHeight, sliderPos - imgHeight * 0.5f);

            g.drawImage(sliderImage,
                        cx, cy, imgWidth, imgHeight,
                        0, 0, sliderImage.getWidth(), sliderImage.getHeight());
        }


    private:
        juce::Image& sliderImage;
    };

    class RotarySliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        RotarySliderLookAndFeel(juce::Image& img) : knobImage(img) {}

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                              float sliderPosProportional, float rotaryStartAngle,
                              float rotaryEndAngle, juce::Slider&) override
        {
            const float angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

            const float cx = x + width * 0.5f;
            const float cy = y + height * 0.5f;

            
            juce::Graphics::ScopedSaveState saved(g);

         
            g.addTransform(juce::AffineTransform::rotation(angle, cx, cy));

       
            const float imageX = cx - (width * 0.5f);
            const float imageY = cy - (height * 0.5f);

            g.drawImage(knobImage, imageX, imageY, width, height, 0, 0,
                        knobImage.getWidth(), knobImage.getHeight());
        }

    private:
        juce::Image knobImage;
    };

    
    class HorizontalSliderLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        HorizontalSliderLookAndFeel(const juce::Image& img) : sliderImage(img) {}

        void drawLinearSlider(juce::Graphics& g,
                              int x, int y, int width, int height,
                              float sliderPos, float minSliderPos, float maxSliderPos,
                              const juce::Slider::SliderStyle style,
                              juce::Slider& slider) override
        {
            if (!sliderImage.isValid())
                return;

            float scale = 0.2f;
            int imgWidth  = static_cast<int>(sliderImage.getWidth() * scale);
            int imgHeight = static_cast<int>(sliderImage.getHeight() * scale);

            float cx = sliderPos - imgWidth * 0.5f;
            float cy = y + (height - imgHeight) * 0.5f;

         
            g.drawImage(sliderImage,
                        cx, cy, imgWidth, imgHeight,
                        0, 0, sliderImage.getWidth(), sliderImage.getHeight());
        }

    private:
        juce::Image sliderImage;
    };


    // --- Sliders & Attachments ---
    juce::Slider inputGainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputGainAttachment;

    juce::Slider outputGainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;

    juce::Slider reverbDryWetSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbDryWetAttachment;

    juce::Slider preDelaySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> preDelayAttachment;

    juce::Slider brightnessSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> brightnessAttachment;

    juce::Slider convMixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> convMixAttachment;

    juce::Slider granularMixSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> granularMixAttachment;

    juce::Slider grainSizeSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> grainSizeAttachment;

    juce::Slider granularFeedbackSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> granularFeedbackAttachment;

    juce::Slider tailStart1Slider, tailEnd1Slider, tailStart2Slider, tailEnd2Slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tailStart1Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tailEnd1Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tailStart2Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> tailEnd2Attachment;

    // --- Labels ---
    juce::Label inputLabel, outputLabel, preDelayLabel;
    juce::Label bpmLabel, ir1LengthLabel, ir2LengthLabel, syncLabel;
    juce::Label brightness, granularMix, grainSize, granularFeedback;
    juce::Label reverbDryWetLabel, convMixLabel;
    juce::Label tailStart1Label, tailEnd1Label, tailStart2Label, tailEnd2Label;

    // --- Buttons & Attachments ---
    juce::ToggleButton syncToBPMButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> syncToBPMAttachment;

    juce::ToggleButton bypassButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassButtonAttachment;

    juce::ComboBox irComboBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> irChoice1Attachment;

    juce::ComboBox irComboBox2;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> irChoice2Attachment;


    std::unique_ptr<VerticalSliderLookAndFeel> inputSliderLAF;
    std::unique_ptr<VerticalSliderLookAndFeel> outputSliderLAF;
    std::unique_ptr<VerticalSliderLookAndFeel> reverbSliderLAF;
    std::unique_ptr<RotarySliderLookAndFeel> preDelayLAF;
    std::unique_ptr<RotarySliderLookAndFeel> brightnessLAF;
    std::unique_ptr<RotarySliderLookAndFeel> granularMixLAF;
    std::unique_ptr<RotarySliderLookAndFeel> grainSizeLAF;
    std::unique_ptr<RotarySliderLookAndFeel> granularFeedbackLAF;
    std::unique_ptr<RotarySliderLookAndFeel> tailStart1LAF;
    std::unique_ptr<RotarySliderLookAndFeel> tailStart2LAF;
    std::unique_ptr<RotarySliderLookAndFeel> tailEnd1LAF;
    std::unique_ptr<RotarySliderLookAndFeel> tailEnd2LAF;
    std::unique_ptr<HorizontalSliderLookAndFeel> convMixLAF;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VERBINPEABODYAudioProcessorEditor)
};


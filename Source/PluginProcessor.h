/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <atomic>

//==============================================================================

class VERBINPEABODYAudioProcessor  : public juce::AudioProcessor,
                                     private juce::Thread,
                                   public juce::AudioProcessorValueTreeState::Listener


{
public:
    //==============================================================================
    VERBINPEABODYAudioProcessor();
    ~VERBINPEABODYAudioProcessor() override;
    bool isPrepared() const noexcept { return prepared; }
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void ReadImpulseResponse(int index, /*bool bypassDecay,*/ float tailStartsSecs, float tailEndSecs);
    void ReadImpulseResponse2(int index,/* bool bypassDecay,*/ float tailStartsSecs, float tailEndSecs);

    void loadImpulseResponse();
    void loadImpulseResponse2();
    void loadSelectedImpulseResponse(int index);
    void loadSelectedImpulseResponse2(int index);
    float getIROriginalLengthSeconds1() const noexcept { return irOriginalLengthSeconds1.load(); }
    float getIROriginalLengthSeconds2() const noexcept { return irOriginalLengthSeconds2.load(); }
    int   getIROriginalSamples1() const noexcept { return irOriginalSamples1.load(); }
    int   getIROriginalSamples2() const noexcept { return irOriginalSamples2.load(); }
    float getTargetDecaySeconds(int convIndex);
    float getTargetPredelayMs();
    void updateConvolution1();
    void updateConvolution2();
    void updateTailParameterRanges(float irLengthSeconds, int irIndex);
    void trimIRTail(juce::AudioBuffer<float>& irBuffer,
                      float tailStartSeconds,
                      float tailEndSeconds,
                      double sampleRate);
    
    void applyTailEnvelopeOnly(juce::AudioBuffer<float>& buffer,
                                                            double sampleRate,
                                                            float tailStartSec,
                               float tailEndSec);
   
    
   juce::ChangeBroadcaster ir1ChangeBroadcaster;
   juce::ChangeBroadcaster ir2ChangeBroadcaster;
    
    juce::UndoManager undoManager;
    juce::AudioProcessorValueTreeState apvts;

    juce::dsp::Convolution convolution;
    juce::dsp::Convolution convolution2;
   
    juce::dsp::DelayLine<float> predelayLine { 48000 };

    // store IR original info for UI
    std::atomic<float> irOriginalLengthSeconds1 { 0.0f };
    std::atomic<float> irOriginalLengthSeconds2 { 0.0f };
    std::atomic<int> irOriginalSamples1 { 0 };
    std::atomic<int> irOriginalSamples2 { 0 };
    
    // For convolution IRs
    std::unique_ptr<juce::AudioBuffer<float>> irBuffer1Temp;
    std::unique_ptr<juce::AudioBuffer<float>> irBuffer2Temp;

    
    std::atomic<bool> ir1Ready { false };
    std::atomic<bool> ir2Ready { false };

    void parameterChanged(const juce::String& parameterID, float newValue) override;
    juce::dsp::ProcessorDuplicator<
        juce::dsp::IIR::Filter<float>,
        juce::dsp::IIR::Coefficients<float>> brightnessFilter;

    struct Grain
    {
        int startSample = 0;
        int lengthSamples = 0;
        int playPos = 0;
        float pan = 0.5f;
        float gain = 1.0f;
    };

    juce::AudioBuffer<float> granularBuffer;
    std::vector<Grain> activeGrains;
    int granularWritePos = 0;
    int maxGrainSamples = 44100 * 2; // 2 seconds at 44.1kHz
    int grainTimeCounter = 0;
    
private:
    bool prepared = false;  
    bool bypassState = false;
    int currentIRIndex1 = 0;
    int currentIRIndex2 = 0;

    std::atomic<bool> shouldReloadIR { false };
    void run() override;
  
    juce::AudioBuffer<float> originalIR1;
    juce::AudioBuffer<float> originalIR2;
    juce::CriticalSection updateLock;
    float previousDecay1 = -1.0f;
    float previousDecay2 = -1.0f;
    bool previousBypass1 = false;
    bool previousBypass2 = false;
  
    std::atomic<bool> needUpdateIR1 { false };
    std::atomic<bool> needUpdateIR2 { false };
    
    //smooth
    juce::LinearSmoothedValue<float> inputGainSmoothed;
    juce::LinearSmoothedValue<float> outputGainSmoothed;
    juce::LinearSmoothedValue<float> preDelaySmoothed;
    juce::LinearSmoothedValue<float> granularMixSmoothed;
    juce::LinearSmoothedValue<float> brightnessSmoothed;
    juce::LinearSmoothedValue<float> convMixSmoothed;
    juce::LinearSmoothedValue<float> reverbDryWetSmoothed;
    juce::LinearSmoothedValue<float> granularFeedbackSmoothed;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VERBINPEABODYAudioProcessor)
};


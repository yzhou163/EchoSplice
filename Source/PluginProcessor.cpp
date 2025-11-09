#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>

using Parameter = juce::AudioProcessorValueTreeState::Parameter;
//==============================================================================
VERBINPEABODYAudioProcessor::VERBINPEABODYAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
      juce::Thread("IR Loader Thread"),
      apvts (*this, nullptr, "PARAMETERS",
                               {
                                   std::make_unique<juce::AudioParameterFloat>("inputGain", "Input Gain",
                                       juce::NormalisableRange<float>(-30.0f, 24.0f, 0.01f), 0.0f),
                                   std::make_unique<juce::AudioParameterFloat>("outputGain", "Output Gain",
                                       juce::NormalisableRange<float>(-60.0f, 24.0f, 0.01f), 0.0f),
                                 
                                   std::make_unique<juce::AudioParameterFloat>("preDelay", "Pre-Delay",
                                       juce::NormalisableRange<float>(30.0f, 100.0f, 0.1f), 0.0f),
        
                                   std::make_unique<juce::AudioParameterFloat>("syncToBPM", "Sync to BPM",
                                       juce::NormalisableRange<float>(0.0f, 1.0f, 1.0f), 0.0f),
    
                                   std::make_unique<juce::AudioParameterFloat>("brightness", "Brightness",
                                       juce::NormalisableRange<float>(-2.0f, 2.0f, 0.01f), 0.0f),
        
                                   std::make_unique<juce::AudioParameterFloat>("granularMix", "Granular Mix",
                                       juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f),
      
                                   std::make_unique<juce::AudioParameterFloat>("grainSize", "Grain Size",
                                       juce::NormalisableRange<float>(100.0f, 2000.0f, 1.0f), 50.0f),

                                   std::make_unique<juce::AudioParameterFloat>("convMix", "Convolution Mix",
                                       juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.0f ),

                                   std::make_unique<juce::AudioParameterFloat>("reverbDryWet", "Reverb Dry/Wet",
                                       juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.8f),
                                  
                                   std::make_unique<juce::AudioParameterFloat>("bypass", "Bypass",
                                       juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f),
          
          std::make_unique<juce::AudioParameterFloat>("tailStart1", "Tail Start 1",
              juce::NormalisableRange<float>(0.0f, 5.0f, 0.001f), 0.0f),

          std::make_unique<juce::AudioParameterFloat>("tailEnd1", "Tail End 1",
              juce::NormalisableRange<float>(0.0f, 5.0f, 0.001f), 0.0f),

          std::make_unique<juce::AudioParameterFloat>("tailStart2", "Tail Start 2",
              juce::NormalisableRange<float>(0.0f, 5.0f, 0.001f),0.00f),

          std::make_unique<juce::AudioParameterFloat>("tailEnd2", "Tail End 2",
              juce::NormalisableRange<float>(0.0f, 5.0f, 0.001f), 0.0f),
          
          std::make_unique<juce::AudioParameterFloat>("granularFeedback", "Granular Feedback",
              juce::NormalisableRange<float>(0.0f, 0.9f), 0.00f),
          
          

          std::make_unique<juce::AudioParameterChoice>(
                 "irChoice1", "IR Choice 1",
                 juce::StringArray{
                     "Hallway 1", "Hallway 2", "MediumStairWay 1", "MediumStairWay 2",
                     "MediumStairWay 3", "MediumStairWay 4", "MediumStairWay5",
                     "Bathroom 1", "Bathroom 2", "Bathroom 3", "Bathroom 4",
                     "Peabody Stair 1", "Peabody Stair 2", "Peabody Stair 3", "Peabody Stair 4",
                     "Friedberg Stair 1", "Friedberg Stair 2", "Friedberg Stair 3", "Friedberg Stair 4",
                     "Friedberg Hall Close1", "Friedberg Hall Close2", "Friedberg Hall Close3",
                     "Friedberg Hall Audience1", "Friedberg Hall Audience2", "Friedberg Hall Audience3"
              
                 },19),
          
          std::make_unique<juce::AudioParameterChoice>(
                  "irChoice2", "IR Choice 2",
                  juce::StringArray{
                      "Hallway 1", "Hallway 2", "MediumStairWay 1", "MediumStairWay 2",
                      "MediumStairWay 3", "MediumStairWay 4", "MediumStairWay5",
                      "Bathroom 1", "Bathroom 2", "Bathroom 3", "Bathroom 4",
                      "Peabody Stair 1", "Peabody Stair 2", "Peabody Stair 3", "Peabody Stair 4",
                      "Friedberg Stair 1", "Friedberg Stair 2", "Friedberg Stair 3", "Friedberg Stair 4",
                      "Friedberg Hall Close1", "Friedberg Hall Close2", "Friedberg Hall Close3",
                      "Friedberg Hall Audience1", "Friedberg Hall Audience2", "Friedberg Hall Audience3"
                      
                  },
                  23
              ),
          

      })
#endif
{
    apvts.addParameterListener("tailStart1", this);
       apvts.addParameterListener("tailEnd1", this);
       apvts.addParameterListener("tailStart2", this);
       apvts.addParameterListener("tailEnd2", this);
       startThread();
}

VERBINPEABODYAudioProcessor::~VERBINPEABODYAudioProcessor()
{
stopThread(2000);
    apvts.removeParameterListener("tailStart1", this);
       apvts.removeParameterListener("tailEnd1", this);
       apvts.removeParameterListener("tailStart2", this);
       apvts.removeParameterListener("tailEnd2", this);
}

const juce::String VERBINPEABODYAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VERBINPEABODYAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VERBINPEABODYAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VERBINPEABODYAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VERBINPEABODYAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VERBINPEABODYAudioProcessor::getNumPrograms()
{
    return 1;
}

int VERBINPEABODYAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VERBINPEABODYAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VERBINPEABODYAudioProcessor::getProgramName (int index)
{
    return {};
}

void VERBINPEABODYAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VERBINPEABODYAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const juce::dsp::ProcessSpec processSpec {
        sampleRate,
        static_cast<juce::uint32>(samplesPerBlock),
        static_cast<juce::uint32>(getTotalNumOutputChannels())
    };
    predelayLine.prepare(processSpec);
    predelayLine.setMaximumDelayInSamples((int)(sampleRate * 0.5));
    convolution.prepare(processSpec);
    convolution2.prepare(processSpec);
    brightnessFilter.prepare({ sampleRate, (juce::uint32)samplesPerBlock, (juce::uint32)getTotalNumOutputChannels() });
    maxGrainSamples = static_cast<int>(2.0f * sampleRate); //max grain size 2 seconds
    granularBuffer.setSize(getTotalNumOutputChannels(), maxGrainSamples);
    granularBuffer.clear();
    granularWritePos = 0;
    activeGrains.clear();
    //smooth
    inputGainSmoothed.reset(sampleRate, 0.005);    // 20ms smoothing
        inputGainSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("inputGain"));

        outputGainSmoothed.reset(sampleRate, 0.005);
        outputGainSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("outputGain"));

        preDelaySmoothed.reset(sampleRate, 0.005);
        preDelaySmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("preDelay"));

        granularMixSmoothed.reset(sampleRate, 0.005);
        granularMixSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("granularMix"));
    
    brightnessSmoothed.reset(sampleRate, 0.005);
    brightnessSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("brightness"));
    
    convMixSmoothed.reset(sampleRate, 0.005);
    convMixSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("convMix"));
    
    reverbDryWetSmoothed.reset(sampleRate, 0.005);
    reverbDryWetSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("reverbDryWet"));
    
    granularFeedbackSmoothed.reset(sampleRate, 0.005);
    granularFeedbackSmoothed.setCurrentAndTargetValue(*apvts.getRawParameterValue("granularFeedback"));
    prepared = true;
}

void VERBINPEABODYAudioProcessor::releaseResources()
{
   
}

//only accept mono or stereo
#ifndef JucePlugin_PreferredChannelConfigurations
bool VERBINPEABODYAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    #endif

    return true;
  #endif
}
#endif

 /*In the ProcessBlock,  I fixed the logic in processBlock to make 2 convolution parallel. It was series before, the convolution2 receives processed signal from convolution1.
Duplicate the input signal into two temporary buffers. Process one buffer through convolution, the other through convolution2.
Mix them back together into the main buffer.*/
void VERBINPEABODYAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                               juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
    
    inputGainSmoothed.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("inputGain")->load()));
       outputGainSmoothed.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("outputGain")->load()));
       preDelaySmoothed.setTargetValue(getTargetPredelayMs());
       granularMixSmoothed.setTargetValue(*apvts.getRawParameterValue("granularMix"));
       brightnessSmoothed.setTargetValue(*apvts.getRawParameterValue("brightness"));
       convMixSmoothed.setTargetValue(*apvts.getRawParameterValue("convMix"));
       reverbDryWetSmoothed.setTargetValue(*apvts.getRawParameterValue("reverbDryWet"));
       granularFeedbackSmoothed.setTargetValue(*apvts.getRawParameterValue("granularFeedback"));
    
//bypass
    auto bypass = *apvts.getRawParameterValue("bypass") > 0.5f;
    if (bypass)
    {
    
        return;
    }
//inputGain
//    buffer.applyGain(inputGainValue);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
       {
           auto* channelData = buffer.getWritePointer(ch);
           for (int i = 0; i < buffer.getNumSamples(); ++i)
               channelData[i] *= inputGainSmoothed.getNextValue();
       }
// buffer for dry/wet
    juce::AudioBuffer<float> dryBuffer;
    dryBuffer.makeCopyOf(buffer);
    
// Predelay
    int delaySamples = static_cast<int>(preDelaySmoothed.getNextValue() * getSampleRate() / 1000.0f);
    predelayLine.setDelay(delaySamples);
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* channelData = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
        {
            float inSample = channelData[i];
            float delayedSample = predelayLine.popSample(ch);
            predelayLine.pushSample(ch, inSample);
            channelData[i] = delayedSample;
        }
    }

//Granular Module
    float granularMix  = granularMixSmoothed.getNextValue();
    float grainSizeMs  = *apvts.getRawParameterValue("grainSize");
    float grainDensity = 80.0f; // 80ms between grains
    float sampleRate   = getSampleRate();
    int numChannels = buffer.getNumChannels();
    int numSamples  = buffer.getNumSamples();
    int grainSizeSamples = juce::jlimit((int)(0.02*sampleRate), maxGrainSamples, int(sampleRate * (grainSizeMs / 1000.0f)));
    int grainIntervalSamples = static_cast<int>(sampleRate * (grainDensity / 1000.0f));

// Write the current signal into a circular buffer
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* in  = buffer.getReadPointer(ch);
        auto* gb  = granularBuffer.getWritePointer(ch);
        float granularFeedback = granularFeedbackSmoothed.getNextValue();

        for (int i = 0; i < numSamples; ++i)
            gb[(granularWritePos + i) % maxGrainSamples] = in[i]  + granularFeedback * gb[(granularWritePos + i) % maxGrainSamples];;
    }
    granularWritePos = (granularWritePos + numSamples) % maxGrainSamples;

// new grains
    grainTimeCounter += numSamples;
    while (grainTimeCounter >= grainIntervalSamples)
    {
        grainTimeCounter -= grainIntervalSamples;

        Grain g;
        g.startSample   = juce::Random::getSystemRandom().nextInt(maxGrainSamples - grainSizeSamples);
        g.lengthSamples = grainSizeSamples;
        g.playPos       = 0;
        g.pan           = juce::Random::getSystemRandom().nextFloat();
        g.gain          = 1.0f;
        activeGrains.push_back(g);
    }

// Mix grains back into buffer
    if (granularMix > 0.001f)
    {
      
        float wetGain = granularMix;
        for (auto it = activeGrains.begin(); it != activeGrains.end(); )
        {
            bool finished = false;
            for (int i = 0; i < numSamples; ++i)
            {
                if (it->playPos >= it->lengthSamples) { finished = true; break; }
                float env = 0.5f * (1 - std::cos(2.0f * juce::MathConstants<float>::pi * it->playPos / it->lengthSamples));
                int readPos = (it->startSample + it->playPos) % maxGrainSamples;
                float panL = std::cos(it->pan * juce::MathConstants<float>::halfPi);
                float panR = std::sin(it->pan * juce::MathConstants<float>::halfPi);
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    auto* buf = buffer.getWritePointer(ch);
                    float grainSample = granularBuffer.getReadPointer(ch)[readPos] * env * wetGain;

                    if (ch == 0) buf[i] += grainSample * panL;
                    else          buf[i] += grainSample * panR;
                }
                it->playPos++;
            }
            if (finished) it = activeGrains.erase(it);
            else ++it;
        }
    }

// Convolution (parallel)
    juce::AudioBuffer<float> convBuffer1(buffer.getNumChannels(), buffer.getNumSamples());
    convBuffer1.makeCopyOf(buffer);
    juce::dsp::AudioBlock<float> convBlock1(convBuffer1);
    convolution.process(juce::dsp::ProcessContextReplacing<float>(convBlock1));
    

    juce::AudioBuffer<float> convBuffer2(buffer.getNumChannels(), buffer.getNumSamples());
    convBuffer2.makeCopyOf(buffer);
    juce::dsp::AudioBlock<float> convBlock2(convBuffer2);
    convolution2.process(juce::dsp::ProcessContextReplacing<float>(convBlock2));

    float convMix = convMixSmoothed.getNextValue();

// Mix conv1 and conv2
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* buf = buffer.getWritePointer(ch);
        auto* conv1 = convBuffer1.getReadPointer(ch);
        auto* conv2 = convBuffer2.getReadPointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            buf[i] = conv1[i] * (1.0f - convMix) + conv2[i] * convMix;
    }
 
// Brightness highshelf
    float brightness = brightnessSmoothed.getNextValue();
    auto coeff = juce::dsp::IIR::Coefficients<float>::makeHighShelf(
        getSampleRate(), 4000.0f, 0.707f, std::pow(2.0f, brightness));
    *brightnessFilter.state = *coeff;
    juce::dsp::AudioBlock<float> finalBlock(buffer);
    juce::dsp::ProcessContextReplacing<float> finalContext(finalBlock);
    brightnessFilter.process(finalContext);
    
//drywetKnob
    float reverbDryWet = reverbDryWetSmoothed.getNextValue();
    float dryGain = 1.0f - reverbDryWet;
    float wetGain = reverbDryWet * 2.0f;

    float total = dryGain + wetGain;
    if (total > 1.0f)
    {
        dryGain /= total;
        wetGain /= total;
    }

// Mix dry wet signal
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* dry = dryBuffer.getReadPointer(ch);
        auto* wet = buffer.getReadPointer(ch);
        auto* out = buffer.getWritePointer(ch);

        for (int i = 0; i < buffer.getNumSamples(); ++i)
            out[i] = dry[i] * dryGain + wet[i] * wetGain;
    }

//volume compensation
    float reverbCompensation = 1.0f + reverbDryWet * 3.0f; // increase total volume as wet increases
    buffer.applyGain(reverbCompensation);

    // Output gain
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
       {
           auto* channelData = buffer.getWritePointer(ch);
           for (int i = 0; i < numSamples; ++i)
               channelData[i] *= outputGainSmoothed.getNextValue();
       }
}


void VERBINPEABODYAudioProcessor::updateTailParameterRanges(float irLengthSeconds, int irIndex)
{
    irLengthSeconds = juce::jmax(0.0f, irLengthSeconds);

    if (irIndex == 1)
    {
        if (auto* startParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("tailStart1")))
            startParam->range = { 0.0f, irLengthSeconds, 0.001f };

      
        if (auto* endParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("tailEnd1")))
        {
            endParam->range = { 0.0f, irLengthSeconds, 0.001f };
        
        }
        ir1ChangeBroadcaster.sendChangeMessage();
        
    }
    else if (irIndex == 2)
    {
        if (auto* startParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("tailStart2")))
            startParam->range = { 0.0f, irLengthSeconds, 0.001f };

        
        if (auto* endParam = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("tailEnd2")))
        {
            endParam->range = { 0.0f, irLengthSeconds, 0.001f };
        }
     
        ir2ChangeBroadcaster.sendChangeMessage();
    }
}


void VERBINPEABODYAudioProcessor::ReadImpulseResponse(int index, /*bool bypassDecay, */float tailStartSecs, float tailEndSecs)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    currentIRIndex1 = index;

    const void* data = nullptr;
    int dataSize = 0;

    switch (index)
    {
        case 0:  data = BinaryData::HallWay1_wav;       dataSize = BinaryData::HallWay1_wavSize; break;
        case 1:  data = BinaryData::HallWay2_wav;       dataSize = BinaryData::HallWay2_wavSize; break;
        case 2:  data = BinaryData::MediumStairWay1_wav; dataSize = BinaryData::MediumStairWay1_wavSize; break;
        case 3:  data = BinaryData::MediumStairWay2_wav; dataSize = BinaryData::MediumStairWay2_wavSize; break;
        case 4:  data = BinaryData::MediumStairWay3_wav; dataSize = BinaryData::MediumStairWay3_wavSize; break;
        case 5:  data = BinaryData::MediumStairWay4_wav; dataSize = BinaryData::MediumStairWay4_wavSize; break;
        case 6:  data = BinaryData::MediumStairWay5_wav; dataSize = BinaryData::MediumStairWay5_wavSize; break;
        case 7:  data = BinaryData::BathRoom1_wav;       dataSize = BinaryData::BathRoom1_wavSize; break;
        case 8:  data = BinaryData::BathRoom2_wav;       dataSize = BinaryData::BathRoom2_wavSize; break;
        case 9:  data = BinaryData::BathRoom3_wav;       dataSize = BinaryData::BathRoom3_wavSize; break;
        case 10: data = BinaryData::BathRoom4_wav;       dataSize = BinaryData::BathRoom4_wavSize; break;
        case 11: data = BinaryData::PeabodyStair1_wav;   dataSize = BinaryData::PeabodyStair1_wavSize; break;
        case 12: data = BinaryData::PeabodyStair2_wav;   dataSize = BinaryData::PeabodyStair2_wavSize; break;
        case 13: data = BinaryData::PeabodyStair3_wav;   dataSize = BinaryData::PeabodyStair3_wavSize; break;
        case 14: data = BinaryData::PeabodyStair4_wav;   dataSize = BinaryData::PeabodyStair4_wavSize; break;
        case 15: data = BinaryData::FriedbergStair1_wav; dataSize = BinaryData::FriedbergStair1_wavSize; break;
        case 16: data = BinaryData::FriedbergStair2_wav; dataSize = BinaryData::FriedbergStair2_wavSize; break;
        case 17: data = BinaryData::FriedbergStair3_wav; dataSize = BinaryData::FriedbergStair3_wavSize; break;
        case 18: data = BinaryData::FriedbergStair4_wav; dataSize = BinaryData::FriedbergStair4_wavSize; break;
        case 19: data = BinaryData::Friedberg_1_Close_WAV; dataSize = BinaryData::Friedberg_1_Close_WAVSize; break;
        case 20: data = BinaryData::Friedberg_2_close_WAV; dataSize = BinaryData::Friedberg_2_close_WAVSize; break;
        case 21: data = BinaryData::Friedberg_3_close_WAV; dataSize = BinaryData::Friedberg_3_close_WAVSize; break;
        case 22: data = BinaryData::MidAudience1_WAV; dataSize = BinaryData::MidAudience1_WAVSize; break;
        case 23: data = BinaryData::midaudience2_WAV; dataSize = BinaryData::midaudience2_WAVSize; break;
        case 24: data = BinaryData::midaudienccelong_wav; dataSize = BinaryData::midaudienccelong_wavSize; break;
    }

    auto inputStream = std::make_unique<juce::MemoryInputStream>(data, static_cast<size_t>(dataSize), false);
    auto formatReader = formatManager.createReaderFor(std::move(inputStream));

    if (formatReader == nullptr)
    {
            DBG("Failed to create formatReader for IR " << index);
            return;
        }
    {
        juce::AudioBuffer<float> irBuffer((int)formatReader->numChannels, (int)formatReader->lengthInSamples);
        formatReader->read(&irBuffer, 0, (int)formatReader->lengthInSamples, 0, true, true);

        // Make stereo if mono
        if (irBuffer.getNumChannels() == 1)
        {
            irBuffer.setSize(2, irBuffer.getNumSamples(), true, true, true);
            irBuffer.copyFrom(1, 0, irBuffer, 0, 0, irBuffer.getNumSamples());
        }

        // Store original IR info for UI
        float origSeconds = static_cast<float>(irBuffer.getNumSamples()) / static_cast<float>(formatReader->sampleRate);
        irOriginalLengthSeconds1.store(origSeconds);
        updateTailParameterRanges(origSeconds, 1);
        irOriginalSamples1.store(irBuffer.getNumSamples());
        originalIR1.makeCopyOf(irBuffer);

        float sampleRate = static_cast<float>(formatReader->sampleRate);

        //  tail range
        int startSample = juce::jlimit(0, irBuffer.getNumSamples(), int(tailStartSecs * sampleRate));
        int endSample   = juce::jlimit(0, irBuffer.getNumSamples(), int(tailEndSecs * sampleRate));
        int numSamples  = endSample - startSample;

        // Slice and apply smooth fade
        for (int ch = 0; ch < irBuffer.getNumChannels(); ++ch)
        {
            auto* dataPtr = irBuffer.getWritePointer(ch, startSample);
            const int fadeSamples = juce::jmin(256, numSamples / 4); // short fade at start/end

            for (int n = 0; n < numSamples; ++n)
            {
                float fade = 1.0f; // default: no change

                // Linear fade-in at the start
                if (n < fadeSamples)
                    fade = float(n) / float(fadeSamples);

                // Linear fade-out at the end
                else if (n >= numSamples - fadeSamples)
                    fade = float(numSamples - n) / float(fadeSamples);

                dataPtr[n] *= fade;
            }
        }

        // Create new buffer with sliced tail
        juce::AudioBuffer<float> tailIR(irBuffer.getNumChannels(), numSamples);
        for (int ch = 0; ch < irBuffer.getNumChannels(); ++ch)
        {
            tailIR.copyFrom(ch, 0, irBuffer, ch, startSample, numSamples);
        }
      

        
        
        convolution.loadImpulseResponse(
            std::move(tailIR),
            formatReader->sampleRate,
            juce::dsp::Convolution::Stereo::yes,
            juce::dsp::Convolution::Trim::yes,
            juce::dsp::Convolution::Normalise::yes
        );
        
     

    }
}


void VERBINPEABODYAudioProcessor::ReadImpulseResponse2(int index, /*bool bypassDecay,*/ float tailStartSecs, float tailEndSecs)
{
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    currentIRIndex2 = index;

    const void* data = nullptr;
    int dataSize = 0;

    switch (index)
    {
            
                case 0:  data = BinaryData::HallWay1_wav;       dataSize = BinaryData::HallWay1_wavSize; break;
                case 1:  data = BinaryData::HallWay2_wav;       dataSize = BinaryData::HallWay2_wavSize; break;
                case 2:  data = BinaryData::MediumStairWay1_wav; dataSize = BinaryData::MediumStairWay1_wavSize; break;
                case 3:  data = BinaryData::MediumStairWay2_wav; dataSize = BinaryData::MediumStairWay2_wavSize; break;
                case 4:  data = BinaryData::MediumStairWay3_wav; dataSize = BinaryData::MediumStairWay3_wavSize; break;
                case 5:  data = BinaryData::MediumStairWay4_wav; dataSize = BinaryData::MediumStairWay4_wavSize; break;
                case 6:  data = BinaryData::MediumStairWay5_wav; dataSize = BinaryData::MediumStairWay5_wavSize; break;
                case 7:  data = BinaryData::BathRoom1_wav;       dataSize = BinaryData::BathRoom1_wavSize; break;
                case 8:  data = BinaryData::BathRoom2_wav;       dataSize = BinaryData::BathRoom2_wavSize; break;
                case 9:  data = BinaryData::BathRoom3_wav;       dataSize = BinaryData::BathRoom3_wavSize; break;
                case 10: data = BinaryData::BathRoom4_wav;       dataSize = BinaryData::BathRoom4_wavSize; break;
                case 11: data = BinaryData::PeabodyStair1_wav;   dataSize = BinaryData::PeabodyStair1_wavSize; break;
                case 12: data = BinaryData::PeabodyStair2_wav;   dataSize = BinaryData::PeabodyStair2_wavSize; break;
                case 13: data = BinaryData::PeabodyStair3_wav;   dataSize = BinaryData::PeabodyStair3_wavSize; break;
                case 14: data = BinaryData::PeabodyStair4_wav;   dataSize = BinaryData::PeabodyStair4_wavSize; break;
                case 15: data = BinaryData::FriedbergStair1_wav; dataSize = BinaryData::FriedbergStair1_wavSize; break;
                case 16: data = BinaryData::FriedbergStair2_wav; dataSize = BinaryData::FriedbergStair2_wavSize; break;
                case 17: data = BinaryData::FriedbergStair3_wav; dataSize = BinaryData::FriedbergStair3_wavSize; break;
                case 18: data = BinaryData::FriedbergStair4_wav; dataSize = BinaryData::FriedbergStair4_wavSize; break;
                case 19: data = BinaryData::Friedberg_1_Close_WAV; dataSize = BinaryData::Friedberg_1_Close_WAVSize; break;
                case 20: data = BinaryData::Friedberg_2_close_WAV; dataSize = BinaryData::Friedberg_2_close_WAVSize; break;
                case 21: data = BinaryData::Friedberg_3_close_WAV; dataSize = BinaryData::Friedberg_3_close_WAVSize; break;
                case 22: data = BinaryData::MidAudience1_WAV; dataSize = BinaryData::MidAudience1_WAVSize; break;
                case 23: data = BinaryData::midaudience2_WAV; dataSize = BinaryData::midaudience2_WAVSize; break;
                case 24: data = BinaryData::midaudienccelong_wav; dataSize = BinaryData::midaudienccelong_wavSize; break;
    }

    auto inputStream = std::make_unique<juce::MemoryInputStream>(data, static_cast<size_t>(dataSize), false);
    auto formatReader = formatManager.createReaderFor(std::move(inputStream));
        if (formatReader == nullptr)
        {
            DBG("Failed to create formatReader for IR " << index);
            return;
        }

    {
        juce::AudioBuffer<float> irBuffer2((int)formatReader->numChannels, (int)formatReader->lengthInSamples);
        formatReader->read(&irBuffer2, 0, (int)formatReader->lengthInSamples, 0, true, true);

        // Convert mono IR to stereo
        if (irBuffer2.getNumChannels() == 1)
        {
            irBuffer2.setSize(2, irBuffer2.getNumSamples(), true, true, true);
            irBuffer2.copyFrom(1, 0, irBuffer2, 0, 0, irBuffer2.getNumSamples());
        }

        // Store original IR info for UI
        float origSeconds = static_cast<float>(irBuffer2.getNumSamples()) / static_cast<float>(formatReader->sampleRate);
        irOriginalLengthSeconds2.store(origSeconds);
        updateTailParameterRanges(origSeconds, 2);
        irOriginalSamples2.store(irBuffer2.getNumSamples());
        originalIR2.makeCopyOf(irBuffer2);

        float sampleRate = static_cast<float>(formatReader->sampleRate);

        // Clamp tail range
        int startSample = juce::jlimit(0, irBuffer2.getNumSamples(), int(tailStartSecs * sampleRate));
        int endSample   = juce::jlimit(0, irBuffer2.getNumSamples(), int(tailEndSecs * sampleRate));
        int numSamples  = endSample - startSample;


        for (int ch = 0; ch < irBuffer2.getNumChannels(); ++ch)
        {
            auto* dataPtr = irBuffer2.getWritePointer(ch, startSample);
            const int fadeSamples = juce::jmin(256, numSamples / 4); // short fade at start/end

            for (int n = 0; n < numSamples; ++n)
            {
                float fade = 1.0f; // default: no change

                // Linear fade-in at the start
                if (n < fadeSamples)
                    fade = float(n) / float(fadeSamples);

                // Linear fade-out at the end
                else if (n >= numSamples - fadeSamples)
                    fade = float(numSamples - n) / float(fadeSamples);

                dataPtr[n] *= fade;
            }
        }


        // Copy the processed tail into a new buffer
        juce::AudioBuffer<float> tailIR(irBuffer2.getNumChannels(), numSamples);
        for (int ch = 0; ch < irBuffer2.getNumChannels(); ++ch)
            tailIR.copyFrom(ch, 0, irBuffer2, ch, startSample, numSamples);
        // Load into convolution
        convolution2.loadImpulseResponse(
            std::move(tailIR),
            formatReader->sampleRate,
            juce::dsp::Convolution::Stereo::yes,
            juce::dsp::Convolution::Trim::yes,
            juce::dsp::Convolution::Normalise::yes
        );



    }
}


void VERBINPEABODYAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "inputGain")
           inputGainSmoothed.setTargetValue(newValue);
       else if (parameterID == "outputGain")
           outputGainSmoothed.setTargetValue(newValue);
       else if (parameterID == "preDelay")
           preDelaySmoothed.setTargetValue(newValue);
       else if (parameterID == "granularMix")
           granularMixSmoothed.setTargetValue(newValue);
       else if (parameterID == "brightness")
           brightnessSmoothed.setTargetValue(newValue);
       else if (parameterID == "convMix")
           convMixSmoothed.setTargetValue(newValue);
       else if (parameterID == "reverbDryWet")
           reverbDryWetSmoothed.setTargetValue(newValue);
       else if (parameterID == "granularFeedback")
           granularFeedbackSmoothed.setTargetValue(newValue);
    
    
    if (
        parameterID == "tailStart1" || parameterID == "tailEnd1")
    {
        needUpdateIR1 = true;
    }
    else if (
             parameterID == "tailStart2" || parameterID == "tailEnd2")
    {
        needUpdateIR2 = true;
    }
}


void VERBINPEABODYAudioProcessor::run()
{
    // Background thread loop: reprocess IRs when parameters change
    while (!threadShouldExit())
    {
        bool updated = false;

        // Check for IR1 update
        if (needUpdateIR1.exchange(false))
        {
            const float tailStart  = *apvts.getRawParameterValue("tailStart1");
            const float tailEnd    = *apvts.getRawParameterValue("tailEnd1");

            ReadImpulseResponse(currentIRIndex1, /*bypassDecay,*/ tailStart, tailEnd);
            updated = true;
        }

        // Check for IR2 update
        if (needUpdateIR2.exchange(false))
        {
            const float tailStart  = *apvts.getRawParameterValue("tailStart2");
            const float tailEnd    = *apvts.getRawParameterValue("tailEnd2");
            ReadImpulseResponse2(currentIRIndex2, /*bypassDecay,*/ tailStart, tailEnd);
            updated = true;
        }

        if (!updated)
            wait(100);
        else
            wait(10);
    }
}

bool VERBINPEABODYAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* VERBINPEABODYAudioProcessor::createEditor()
{
    return new VERBINPEABODYAudioProcessorEditor (*this);
}

//==============================================================================

void VERBINPEABODYAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
  
}

void VERBINPEABODYAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
 
}

//==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VERBINPEABODYAudioProcessor();
}


float VERBINPEABODYAudioProcessor::getTargetDecaySeconds(int convIndex)
{
    constexpr float minDecay = 0.1f;
    constexpr float maxDecay = 2.0f;

    bool syncToBPM = apvts.getRawParameterValue("syncToBPM")->load() > 0.5f;


    double bpm = 120.0;
    if (auto* playHead = getPlayHead())
        if (auto* playHead = getPlayHead())
          {
              if (auto position = playHead->getPosition())
              {
                  if (auto bpmOpt = position->getBpm())
                  {
                      double newBpm = *bpmOpt;
                      if (newBpm > 0.0)
                          bpm = newBpm;
                  }
              }
          }

    float decaySeconds = 1.0f;
    if (syncToBPM)
    {
        // 1 bar = 4 beats, scale from 0.1–2.0 slider as a multiplier
        float decayKnob = 0.5f;
        decaySeconds = static_cast<float>((60.0 / bpm) * 4.0 * decayKnob);
        decaySeconds = juce::jlimit(minDecay, maxDecay, decaySeconds);
        if (convIndex == 1)
            *apvts.getRawParameterValue("decay1") = decaySeconds;
        else if (convIndex == 2)
            *apvts.getRawParameterValue("decay2") = decaySeconds;
    }
    else
    {
        if (convIndex == 1)
            decaySeconds = apvts.getRawParameterValue("decay1")->load();
        else if (convIndex == 2)
            decaySeconds = apvts.getRawParameterValue("decay2")->load();

        decaySeconds = juce::jlimit(minDecay, maxDecay, decaySeconds);
    }

    return decaySeconds;
}


float VERBINPEABODYAudioProcessor::getTargetPredelayMs()
{
    bool syncToBPM = apvts.getRawParameterValue("syncToBPM")->load() > 0.5f;

    float predelayMs = apvts.getRawParameterValue("preDelay")->load(); // default/manual

    if (syncToBPM)
    {
        double bpm = 120.0;
        if (auto* playHead = getPlayHead())
          {
              if (auto position = playHead->getPosition())
              {
                  if (auto bpmOpt = position->getBpm())
                  {
                      double newBpm = *bpmOpt;
                      if (newBpm > 0.0)
                          bpm = newBpm;
                  }
              }
          }
        predelayMs = static_cast<float>((60.0 / bpm) * 1000.0 / 12.0); // quarter note divided by 4 = 16th
        predelayMs = juce::jlimit(30.0f, 100.0f, predelayMs);
        *apvts.getRawParameterValue("preDelay") = predelayMs;
    }
    return predelayMs;
}

void VERBINPEABODYAudioProcessor::updateConvolution1()
{
    float tailStart = apvts.getRawParameterValue("tailStart1")->load();
    float tailEnd   = apvts.getRawParameterValue("tailEnd1")->load();
    
    
    juce::AudioBuffer<float> tempIR = originalIR1;
    trimIRTail(tempIR, tailStart, tailEnd, getSampleRate());
    convolution.loadImpulseResponse(
        std::move(tempIR),
        getSampleRate(),
        juce::dsp::Convolution::Stereo::yes,
        juce::dsp::Convolution::Trim::yes,
        juce::dsp::Convolution::Normalise::yes
    );
}

void VERBINPEABODYAudioProcessor::updateConvolution2()
{
    float tailStart = apvts.getRawParameterValue("tailStart2")->load();
    float tailEnd   = apvts.getRawParameterValue("tailEnd2")->load();
    
    juce::AudioBuffer<float> tempIR = originalIR2;
    trimIRTail(tempIR, tailStart, tailEnd, getSampleRate());
    convolution2.loadImpulseResponse(
        std::move(tempIR),
        getSampleRate(),
        juce::dsp::Convolution::Stereo::yes,
        juce::dsp::Convolution::Trim::yes,
        juce::dsp::Convolution::Normalise::yes
    );
}

void VERBINPEABODYAudioProcessor::trimIRTail(juce::AudioBuffer<float>& irBuffer,
                float tailStartSeconds,
                float tailEndSeconds,
                double sampleRate)
{
    int startSample = static_cast<int>(tailStartSeconds * sampleRate);
    int endSample   = static_cast<int>(tailEndSeconds * sampleRate);

    startSample = juce::jlimit(0, irBuffer.getNumSamples() - 1, startSample);
    endSample   = juce::jlimit(startSample, irBuffer.getNumSamples(), endSample);

    int numChannels = irBuffer.getNumChannels();
    for (int ch = 0; ch < numChannels; ++ch)
    {
        float* data = irBuffer.getWritePointer(ch);

        
        for (int i = 0; i < startSample; ++i)
            data[i] = 0.0f;

    
        for (int i = endSample; i < irBuffer.getNumSamples(); ++i)
            data[i] = 0.0f;
    }
}
void VERBINPEABODYAudioProcessor::applyTailEnvelopeOnly(juce::AudioBuffer<float>& buffer,
                                                        double sampleRate,
                                                        float tailStartSec,
                                                        float tailEndSec)
{
    int numSamples = buffer.getNumSamples();
    int startSample = juce::jlimit(0, numSamples, int(tailStartSec * sampleRate));
    int endSample   = juce::jlimit(0, numSamples, int(tailEndSec   * sampleRate));

    if (endSample <= startSample)
        return;
    // apply a smooth fade envelope between tailStartSec and tailEndSec
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
    {
        auto* data = buffer.getWritePointer(ch);
        for (int i = 0; i < numSamples; ++i)
        {
            if (i < startSample)
                data[i] = 0.0f;
            else if (i >= startSample && i <= endSample)
            {
                float t = float(i - startSample) / float(endSample - startSample);
                float env = 0.5f * (1.0f - std::cos(juce::MathConstants<float>::pi * t)); // smooth window
                data[i] *= env;
            }
            else
                data[i] = 0.0f;
        }
    }
}



/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ChorusModulatorsAndStatefulnessAudioProcessor::ChorusModulatorsAndStatefulnessAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ChorusModulatorsAndStatefulnessAudioProcessor::~ChorusModulatorsAndStatefulnessAudioProcessor()
{
    addParameter(mDryWetParameter = new AudioParameterFloat("drywet", "Dry Wet", 0.0, 1.0, 0.5));
    addParameter(mDepthParameter = new AudioParameterFloat("depth", "Depth", 0.0, 1.0, 0.5));
    addParameter(mRateParameter = new AudioParameterFloat("rate", "Rate", 0.1f, 20.f, 10.f));
    addParameter(mPhaseOffsetParameter = new AudioParameterFloat("phaseoffset", "Phase Offset", 0.0f, 1.f, 0.f));
    addParameter(mFeedbackParameter = new AudioParameterFloat("feedback", "Feedback", 0.0, 0.98, 0.5));
    addParameter(mTypeParameter = new AudioParameterInt("type", "Type", 0, 1, 0));
    
    mCircularBufferWriteHead = 0;
    mCircularBufferLeft = nullptr;
    mCircularBufferRight = nullptr;
    mCircularBufferLength = 0;
    mDelayTimeInSamples = 0;
    mDelayReadHead = 0;
    mFeedbackLeft = 0;
    mFeedbackRight = 0;
    mDelayTimeSmoothed = 0;
    
    mLFOPhase = 0;

}

//==============================================================================
const String ChorusModulatorsAndStatefulnessAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ChorusModulatorsAndStatefulnessAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ChorusModulatorsAndStatefulnessAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ChorusModulatorsAndStatefulnessAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ChorusModulatorsAndStatefulnessAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ChorusModulatorsAndStatefulnessAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ChorusModulatorsAndStatefulnessAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ChorusModulatorsAndStatefulnessAudioProcessor::setCurrentProgram (int index)
{
}

const String ChorusModulatorsAndStatefulnessAudioProcessor::getProgramName (int index)
{
    return {};
}

void ChorusModulatorsAndStatefulnessAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ChorusModulatorsAndStatefulnessAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    mDelayTimeInSamples = 1;
    mLFOPhase = 0;
    
    mCircularBufferLength = sampleRate * MAX_DELAY_TIME;

    if (mCircularBufferLeft == nullptr) {
        mCircularBufferLeft = new float[mCircularBufferLength];
    }

    zeromem(mCircularBufferLeft, mCircularBufferLength * sizeof(float));

    if (mCircularBufferRight == nullptr) {
        mCircularBufferRight = new float[mCircularBufferLength];
    }

    zeromem(mCircularBufferRight, mCircularBufferLength * sizeof(float));

    mCircularBufferWriteHead = 0;
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void ChorusModulatorsAndStatefulnessAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    if (mCircularBufferLeft != nullptr)
    {
        delete [] mCircularBufferLeft;
        mCircularBufferLeft = nullptr;
    }
    
    if (mCircularBufferRight != nullptr)
    {
        delete [] mCircularBufferRight;
        mCircularBufferRight = nullptr;
    }

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ChorusModulatorsAndStatefulnessAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ChorusModulatorsAndStatefulnessAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    float* leftChannel = buffer.getWritePointer(0);
    float* rightChannel = buffer.getWritePointer(1);
    
    for (int i = 0; i < buffer.getNumSamples(); i++) {
        
        float lfoOut = sin(2*M_PI * mLFOPhase);
        
        mLFOPhase += 50 * getSampleRate();
        
        if (mLFOPhase > 1) {
            mLFOPhase -= 1;
        }
        
        float lfoOutMapped = jmap((float)lfoOut, -1.f, 1.f, 0.005f, 0.03f);
        
        
        mDelayTimeSmoothed = mDelayTimeSmoothed - 0.001 * (mDelayTimeSmoothed - lfoOutMapped);
        mDelayTimeInSamples = getSampleRate() * mDelayTimeSmoothed;
        
        mCircularBufferLeft[mCircularBufferWriteHead] = leftChannel[i] + mFeedbackLeft;
        mCircularBufferRight[mCircularBufferWriteHead] = rightChannel[i] + mFeedbackRight;
        
        mDelayReadHead = mCircularBufferWriteHead - mDelayTimeInSamples;
        
        if (mDelayReadHead < 0) {
            mDelayReadHead += mCircularBufferLength;
        }
        
        int readHead_x = (int)mDelayReadHead;
        int readHead_x1 = readHead_x + 1;
        float readHeadFloat = mDelayReadHead - readHead_x;
        
        if (readHead_x1 >= mCircularBufferLength) {
            readHead_x1 -= mCircularBufferLength;
        }
        
        float delay_sample_left = lin_interp(mCircularBufferLeft[readHead_x], mCircularBufferLeft[readHead_x1], readHeadFloat);
        float delay_sample_right = lin_interp(mCircularBufferRight[readHead_x], mCircularBufferRight[readHead_x1], readHeadFloat);
        
        mFeedbackLeft = delay_sample_left * 0.5f;
        mFeedbackRight = delay_sample_right * 0.5f;
        
        mCircularBufferWriteHead++;
        
        buffer.setSample(0, i,
                         buffer.getSample(0, i) * (1 - 0.5) + delay_sample_left * 0.5);
        buffer.setSample(1, i,
                         buffer.getSample(1, i) * (1 - 0.5) + delay_sample_right *  0.5);
        
        if (mCircularBufferWriteHead >= mCircularBufferLength) {
            mCircularBufferWriteHead = 0;
        }
    }}

//==============================================================================
bool ChorusModulatorsAndStatefulnessAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ChorusModulatorsAndStatefulnessAudioProcessor::createEditor()
{
    return new ChorusModulatorsAndStatefulnessAudioProcessorEditor (*this);
}

//==============================================================================
void ChorusModulatorsAndStatefulnessAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ChorusModulatorsAndStatefulnessAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

float ChorusModulatorsAndStatefulnessAudioProcessor::lin_interp(float sample_x, float sample_x1, float inPhase)
{
    return (1 - inPhase) * sample_x + inPhase * sample_x1;
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ChorusModulatorsAndStatefulnessAudioProcessor();
}

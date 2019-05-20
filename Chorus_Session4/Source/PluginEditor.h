/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Chorus_session4AudioProcessorEditor  : public AudioProcessorEditor
{
public:
    Chorus_session4AudioProcessorEditor (Chorus_session4AudioProcessor&);
    ~Chorus_session4AudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Chorus_session4AudioProcessor& processor;

    Slider mDryWetSlider;
    Slider mDepthSlider;
    Slider mRateSlider;
    Slider mPhaseOffsetSlider;
    Slider mFeedbackSlider;
    
    ComboBox mType;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Chorus_session4AudioProcessorEditor)
};

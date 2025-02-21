/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class IngitionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    IngitionAudioProcessorEditor(IngitionAudioProcessor&);
    ~IngitionAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    IngitionAudioProcessor& audioProcessor;

    juce::Slider driveSlider, toneSlider, mixSlider;

    juce::Slider envelopeSlider, envelopeDriveSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment, toneAttachment, mixAttachment;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> envelopeAttachment, envelopeDriveAttachment;

    juce::ComboBox distortionTypeSelector;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> distortionTypeAttachment;

    // Pre/Post Toggle Buttons
    juce::ToggleButton preButton{ "Pre" };
    juce::ToggleButton postButton{ "Post" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> preAttachment, postAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IngitionAudioProcessorEditor)
};
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

    // Filter
    juce::Slider cutoffSlider, resonanceSlider, cutoffModSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> cutoffAttachment, resonanceAttachment, cutoffModAttachment;

    juce::ToggleButton preButton{ "Pre" };
    juce::ToggleButton postButton{ "Post" };

    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> preAttachment, postAttachment;

    // Distortion
    juce::Slider driveSlider, driveModSlider; // mixSlide;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment, driveModAttachment;

    juce::ComboBox distortionTypeSelector;

    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> distortionTypeAttachment;

    // Other
    juce::Slider mixSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixAttachment;

    juce::Slider gateSlider;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gateAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IngitionAudioProcessorEditor)
};
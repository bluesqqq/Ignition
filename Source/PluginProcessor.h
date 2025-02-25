/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <juce_dsp/juce_dsp.h>
#include "EnvelopeFollower.h"
#include "DistortionEngine.h"

using namespace juce;
//==============================================================================
/**
*/
class IngitionAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    IngitionAudioProcessor();
    ~IngitionAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif
    std::vector<float>& getEnvelopeHistory();
    std::vector<float>& getEnvelope2History();
    std::vector<float> getWaveshape();
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

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
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    AudioProcessorValueTreeState apvts;

private:
    //==============================================================================
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float lastSampleRate;

    dsp::StateVariableTPTFilter<float> preFilter, postFilter;

    DistortionEngine distortion;

    EnvelopeFollower envelopeFollower, envelopeFollower2;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IngitionAudioProcessor)
};

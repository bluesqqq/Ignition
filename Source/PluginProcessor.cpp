/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#define _USE_MATH_DEFINES

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
IngitionAudioProcessor::IngitionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ), apvts(*this, nullptr, "PARAMETERS", createParameterLayout()),
    distortion(DistortionEngine())
#endif
{
}

juce::AudioProcessorValueTreeState::ParameterLayout IngitionAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Pre Filter
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pre-filter cutoff",     "Pre-Filter Cutoff",     0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pre-filter resonance",  "Pre-Filter Resonance",  0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pre-filter cutoff mod", "Pre-Filter Cutoff Mod", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("pre-filter on", "Pre-Filter On", false));

    // Post Filter
    params.push_back(std::make_unique<juce::AudioParameterFloat>("post-filter cutoff",     "Post-Filter Cutoff",     0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("post-filter resonance",  "Post-Filter Resonance",  0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("post-filter cutoff mod", "Post-Filter Cutoff Mod", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("post-filter on", "Post-Filter On", false));

    // Distortion
    params.push_back(std::make_unique<juce::AudioParameterFloat>("drive", "Drive", 0.01f, 20.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("drive mod", "Drive Mod", 0.0f, 1.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("distortion type", "Distortion Type", juce::StringArray{ "Hard Clip", "Tube", "Fuzz", "Rectify", "Downsample" }, 0));

    params.push_back(std::make_unique<juce::AudioParameterFloat>("mix", "Mix", 0.0f, 1.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("gate", "Gate", 0.0f, 1.0f, 0.0f));


    return { params.begin(), params.end() };
}

IngitionAudioProcessor::~IngitionAudioProcessor()
{
}

//==============================================================================
const juce::String IngitionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool IngitionAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool IngitionAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool IngitionAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double IngitionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int IngitionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int IngitionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void IngitionAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String IngitionAudioProcessor::getProgramName(int index)
{
    return {};
}

void IngitionAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void IngitionAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    lastSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();

    preFilter.reset();
    preFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    preFilter.setCutoffFrequency(20000.0f);

    postFilter.reset();
    postFilter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
    postFilter.setCutoffFrequency(20000.0f);
}

void IngitionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool IngitionAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
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

std::vector<float>& IngitionAudioProcessor::getEnvelopeHistory()
{
    return envelopeFollower.getEnvelopeHistory();
}

std::vector<float>& IngitionAudioProcessor::getEnvelope2History()
{
    return envelopeFollower2.getEnvelopeHistory();
}

std::vector<float> IngitionAudioProcessor::getWaveshape() {
    return distortion.getWaveshape();
}

void IngitionAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Filter parameters
    float pPreFilterCutoff    = apvts.getRawParameterValue("pre-filter cutoff")->load();
    float pPreFilterResonance = apvts.getRawParameterValue("pre-filter resonance")->load();
    float pPreFilterCutoffMod = apvts.getRawParameterValue("pre-filter cutoff mod")->load();
    bool pPreFilterOn         = apvts.getRawParameterValue("pre-filter on")->load() > 0.5f;

    float pPostFilterCutoff    = apvts.getRawParameterValue("post-filter cutoff")->load();
    float pPostFilterResonance = apvts.getRawParameterValue("post-filter resonance")->load();
    float pPostFilterCutoffMod = apvts.getRawParameterValue("post-filter cutoff mod")->load();
    bool pPostFilterOn         = apvts.getRawParameterValue("post-filter on");

    // Distortion parameters
    float pDrive          = apvts.getRawParameterValue("drive")->load();
    float pDriveMod       = apvts.getRawParameterValue("drive mod")->load();
    int   pDistortionType = apvts.getRawParameterValue("distortion type")->load();

    // Other parameters
    float pMix = apvts.getRawParameterValue("mix")->load();

    // Envelope parameters
    float pGate = apvts.getRawParameterValue("gate")->load();

    preFilter.setResonance(juce::jmap(pPreFilterResonance, 0.707f, 4.0f));
    postFilter.setResonance(juce::jmap(pPostFilterResonance, 0.707f, 4.0f));
    float preFilterCutoff  = juce::jmap(pPreFilterCutoff, 200.0f, 20000.0f);
    float postFilterCutoff = juce::jmap(pPostFilterCutoff, 200.0f, 20000.0f);

    const float maxCutoff = 0.45f * lastSampleRate;

    envelopeFollower.setSampleRate(lastSampleRate);
    envelopeFollower2.setSampleRate(lastSampleRate);
    envelopeFollower.setGate(pGate);

    // Set the distortion parameters
    distortion.setDistortionAlgorithm(pDistortionType);
    distortion.setDrive(pDrive);

    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            //=============// CLEAN SIGNAL //=============//
            float drySignal = channelData[sample]; // Clean signal

            float wetSignal = drySignal; // The signal that affects will be applied to

            float envDry = envelopeFollower.process(wetSignal); // Update the envelope based on the current sample

            //=======// PRE-DISTORTION FILTERING //=======//
            if (pPreFilterOn)
            {
                const float modulatedPreFilterCutoff = std::min(preFilterCutoff + (20000.0f * envDry * pPreFilterCutoffMod), maxCutoff);
                preFilter.setCutoffFrequency(modulatedPreFilterCutoff);

                wetSignal = preFilter.processSample(channel, wetSignal);
            }

            //==============// DISTORTION //==============//
            distortion.setModulation(envDry * pDriveMod); // Set the modulation

            wetSignal = distortion.processSample(wetSignal); // Distort signal

            //=======// POST-DISTORTION FILTERING //======//
            if (pPostFilterOn)
            {
                const float modulatedPostFilterCutoff = std::min(postFilterCutoff + (20000.0f * envDry * pPostFilterCutoffMod), maxCutoff);
                postFilter.setCutoffFrequency(modulatedPostFilterCutoff);

                wetSignal = postFilter.processSample(channel, wetSignal);
            }

            //==============// DRY-WET MIX //=============//
            float mixSignal = juce::jmap(pMix, drySignal, wetSignal); // Dry-wet mixed signal

            channelData[sample] = mixSignal; // Example: Apply simple gain reduction

            envelopeFollower2.process(mixSignal);
        }
    }
}

//==============================================================================
bool IngitionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* IngitionAudioProcessor::createEditor()
{
    return new IngitionAudioProcessorEditor(*this);
}

//==============================================================================
void IngitionAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void IngitionAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new IngitionAudioProcessor();
}

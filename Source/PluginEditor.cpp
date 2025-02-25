/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
IngitionAudioProcessorEditor::IngitionAudioProcessorEditor(IngitionAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(500, 600);

    // Pre Filter
    preFilterCutoffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    preFilterCutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(preFilterCutoffSlider);
    preFilterCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "pre-filter cutoff", preFilterCutoffSlider);

    preFilterResonanceSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    preFilterResonanceSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(preFilterResonanceSlider);
    preFilterResonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "pre-filter resonance", preFilterResonanceSlider);

    preFilterCutoffModSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    preFilterCutoffModSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(preFilterCutoffModSlider);
    preFilterCutoffModAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "pre-filter cutoff mod", preFilterCutoffModSlider);
    
    addAndMakeVisible(preFilterOnButton);
    preFilterOnButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "pre-filter on", preFilterOnButton);

    // Post Filter
    postFilterCutoffSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    postFilterCutoffSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(postFilterCutoffSlider);
    postFilterCutoffAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "post-filter cutoff", postFilterCutoffSlider);

    postFilterResonanceSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    postFilterResonanceSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(postFilterResonanceSlider);
    postFilterResonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "post-filter resonance", postFilterResonanceSlider);

    postFilterCutoffModSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    postFilterCutoffModSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(postFilterCutoffModSlider);
    postFilterCutoffModAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "post-filter cutoff mod", postFilterCutoffModSlider);

    addAndMakeVisible(postFilterOnButton);
    postFilterOnButtonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.apvts, "post-filter on", postFilterOnButton);
    

    // Drive
    driveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(driveSlider);
    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "drive", driveSlider);

    driveModSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driveModSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(driveModSlider);
    driveModAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "drive mod", driveModSlider);

    distortionTypeSelector.addItem("Hard Clip", 1);
    distortionTypeSelector.addItem("Tube", 2);
    distortionTypeSelector.addItem("Fuzz", 3);
    distortionTypeSelector.addItem("Rectify", 4);
    distortionTypeSelector.addItem("Downsample", 5);
    addAndMakeVisible(distortionTypeSelector);
    distortionTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "distortion type", distortionTypeSelector);

    // Other
    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(mixSlider);
    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mix", mixSlider);

    // Envelope
    gateSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    gateSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(gateSlider);
    gateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "gate", gateSlider);

}

IngitionAudioProcessorEditor::~IngitionAudioProcessorEditor()
{
}

//==============================================================================
void IngitionAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);

    const float width = getWidth();
    const float height = getHeight();

    // Define the area where the envelope history is drawn
    const float envelopeAreaHeight = 100.0f;  // Adjust this height based on your layout
    const juce::Rectangle<float> envelopeBounds(0, height - envelopeAreaHeight, width, envelopeAreaHeight);

    // Optional: Fill the background for the envelope area
    g.setColour(juce::Colours::black);
    g.fillRect(envelopeBounds);


    const auto& envelope2History = audioProcessor.getEnvelope2History();
    const int history2Size = envelope2History.size();

    if (history2Size > 1)
    {
        g.setColour(juce::Colours::red);

        const float stepX = width / static_cast<float>(history2Size);

        // Draw the envelope history as lines
        for (int i = 1; i < history2Size; ++i)
        {
            const float x1 = (i - 1) * stepX;
            const float y1 = envelopeBounds.getBottom() - envelope2History[i - 1] * 100.0f; // Adjust scaling factor as needed
            const float x2 = i * stepX;
            const float y2 = envelopeBounds.getBottom() - envelope2History[i] * 100.0f;

            g.drawLine(x1, y1, x2, y2, 2.0f); // Line width of 2.0f
        }
    }

    const auto& envelopeHistory = audioProcessor.getEnvelopeHistory();
    const int historySize = envelopeHistory.size();

    if (historySize > 1)
    {
        g.setColour(juce::Colours::white);

        const float stepX = width / static_cast<float>(historySize);

        // Draw the envelope history as lines
        for (int i = 1; i < historySize; ++i)
        {
            const float x1 = (i - 1) * stepX;
            const float y1 = envelopeBounds.getBottom() - envelopeHistory[i - 1] * 100.0f; // Adjust scaling factor as needed
            const float x2 = i * stepX;
            const float y2 = envelopeBounds.getBottom() - envelopeHistory[i] * 100.0f;

            g.drawLine(x1, y1, x2, y2, 2.0f); // Line width of 2.0f
        }
    }

    // DRAW THE DISTORTION WAVETABLE!!!
    const auto waveshapePoints = audioProcessor.getWaveshape();

    g.setColour(juce::Colours::white);

    const float wavetableWidth = 100.0f;
    const float wavetableHeight = 100.0f;
    const float halfHeight = wavetableHeight / 2;
    const float wavetableX = 150;
    const float wavetableY = 100;

    const float stepX = wavetableWidth / static_cast<float>(waveshapePoints.size());

    for (int i = 1; i < waveshapePoints.size(); ++i)
    {
        const float x1 = wavetableX + (i - 1) * stepX;
        const float y1 = wavetableY + halfHeight + waveshapePoints[i - 1] * -halfHeight; // Adjust scaling factor as needed
        const float x2 = wavetableX + i * stepX;
        const float y2 = wavetableY + halfHeight + waveshapePoints[i] * -halfHeight;

        g.drawLine(x1, y1, x2, y2, 2.0f); // Line width of 2.0f
    }

    // Trigger repaint for the envelope area, in case it’s being blocked by another component like a dial
    // The repaint request can be called here if necessary, or when the envelope history changes
    repaint(envelopeBounds.toNearestInt()); // Repaint only the area containing the envelope lines
    repaint(Rectangle<int>(wavetableX, wavetableY, wavetableWidth, wavetableHeight)); // Repaint only the area containing the envelope lines
}


void IngitionAudioProcessorEditor::resized()
{
    // Filter
    preFilterCutoffSlider.setBounds(0, 50, 100, 100);
    preFilterResonanceSlider.setBounds(0, 150, 100, 100);
    preFilterCutoffModSlider.setBounds(0, 250, 100, 100);
    preFilterOnButton.setBounds(25, 0, 50, 50);

    postFilterCutoffSlider.setBounds(400, 50, 100, 100);
    postFilterResonanceSlider.setBounds(400, 150, 100, 100);
    postFilterCutoffModSlider.setBounds(400, 250, 100, 100);
    postFilterOnButton.setBounds(425, 0, 50, 50);

    // Distortion
    driveSlider.setBounds(150, 50, 200, 200);
    driveModSlider.setBounds(200, 250, 100, 100);
    distortionTypeSelector.setBounds(200, 350, 100, 40);

    // Other
    mixSlider.setBounds(400, 400, 100, 100);

    // Envelope
    gateSlider.setBounds(300, 400, 100, 100);
}

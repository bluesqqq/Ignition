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
    setSize(400, 500);

    driveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    driveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(driveSlider);

    driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "drive", driveSlider);

    toneSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    toneSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(toneSlider);

    toneAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "tone", toneSlider);

    envelopeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    envelopeSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(envelopeSlider);

    envelopeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "envelope", envelopeSlider);

    envelopeDriveSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    envelopeDriveSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(envelopeDriveSlider);

    envelopeDriveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "envelope drive", envelopeDriveSlider);

    // Set mutual exclusivity (Only one button can be selected at a time)
    preButton.setRadioGroupId(1);
    preButton.setButtonText("Pre");
    postButton.setRadioGroupId(1);
    postButton.setButtonText("Post");

    // Read initial state from AudioProcessorValueTreeState
    int routing = *audioProcessor.apvts.getRawParameterValue("filter routing");
    preButton.setToggleState(routing == 0, juce::dontSendNotification);
    postButton.setToggleState(routing == 1, juce::dontSendNotification);

    // Button Listeners to update parameter choice
    preButton.onClick = [this] { audioProcessor.apvts.getParameter("filter routing")->setValueNotifyingHost(0); };
    postButton.onClick = [this] { audioProcessor.apvts.getParameter("filter routing")->setValueNotifyingHost(1); };

    // Add buttons to UI
    addAndMakeVisible(preButton);
    addAndMakeVisible(postButton);

    mixSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    mixSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    addAndMakeVisible(mixSlider);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "mix", mixSlider);

    // Distortion Type selector
    distortionTypeSelector.addItem("Hard Clip", 1);
    distortionTypeSelector.addItem("Tube", 2);
    distortionTypeSelector.addItem("Fuzz", 3);
    distortionTypeSelector.addItem("Rectify", 4);
    distortionTypeSelector.addItem("Downsample", 5);
    addAndMakeVisible(distortionTypeSelector);

    distortionTypeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts, "distortion type", distortionTypeSelector);
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
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    driveSlider.setBounds(100, 50, 200, 200);
    toneSlider.setBounds(0, 50, 100, 100);
    envelopeSlider.setBounds(0, 150, 100, 100);
    envelopeDriveSlider.setBounds(150, 250, 100, 100);

    mixSlider.setBounds(300, 100, 100, 100);

    distortionTypeSelector.setBounds(150, 350, 100, 40);

    preButton.setBounds(0, 250, 100, 25);
    postButton.setBounds(0, 275, 100, 25);
}

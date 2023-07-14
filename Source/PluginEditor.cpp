/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleDelayAudioProcessorEditor::SimpleDelayAudioProcessorEditor(SimpleDelayAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), freqAT(audioProcessor.apvts, "freq", freq),
    feedbackAT(audioProcessor.apvts, "feedback", feedback), dryWetAT(audioProcessor.apvts, "dryWet", dryWet)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    freq.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    freq.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(freq);

    feedback.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedback.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(feedback);

    dryWet.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dryWet.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(dryWet);

    setSize (400, 300);
}

SimpleDelayAudioProcessorEditor::~SimpleDelayAudioProcessorEditor()
{
}

//==============================================================================
void SimpleDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    //g.drawFittedText("Hello", getLocalBounds(), juce::Justification::bottom, 1);
}

void SimpleDelayAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();

    auto freqBounds = bounds.removeFromLeft(bounds.getWidth() * .33);
    freq.setBounds(freqBounds);

    auto feedbackBounds = bounds.removeFromLeft(bounds.getWidth() * .5);
    feedback.setBounds(feedbackBounds);

    auto dryWetBounds = bounds.removeFromLeft(bounds.getWidth());
    dryWet.setBounds(dryWetBounds);
}

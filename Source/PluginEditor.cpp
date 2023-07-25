/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

/*TODO;
* Create custom Link Button
* Add Second button to allow for different wet algo
* update value boxes
* Change it so that the dials snap together on true
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleDelayAudioProcessorEditor::SimpleDelayAudioProcessorEditor(SimpleDelayAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), freqLeftAT(audioProcessor.apvts, "freqLeft", freqLeft),
    freqRightAT(audioProcessor.apvts, "freqRight", freqRight), feedbackAT(audioProcessor.apvts, "feedback", feedback), 
    dryWetAT(audioProcessor.apvts, "dryWet", dryWet), linkAT(audioProcessor.apvts, "link", link)
{
    setLookAndFeel(&laf);

    addAndMakeVisible(meterL);
    addAndMakeVisible(meterR);
    addAndMakeVisible(outMeterL);
    addAndMakeVisible(outMeterR);

    freqLeft.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    freqLeft.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(freqLeft);

    freqRight.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    freqRight.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(freqRight);

    feedback.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedback.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(feedback);

    dryWet.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dryWet.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 100, 20);
    addAndMakeVisible(dryWet);

    link.setToggleState(true, juce::dontSendNotification);
    link.setButtonText("Link");
    addAndMakeVisible(link);

    freqLeft.onValueChange = [this]()
    {
        if (link.getToggleState()) {
            freqRight.setValue(freqLeft.getValue());
        }
        else{}
    };

    freqRight.onValueChange = [this]()
    {
        if (link.getToggleState()) {
            freqLeft.setValue(freqRight.getValue());
        }
        else {}
    };

    link.onClick = [this]()
    {
        if (link.getToggleState()) {
            freqRight.setValue(freqLeft.getValue());
        }
    };

    startTimerHz(24);

    setSize (600, 400);
}

SimpleDelayAudioProcessorEditor::~SimpleDelayAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
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

    auto inputMeter = bounds.removeFromLeft(bounds.getWidth() * .167);
    auto meterLSide = inputMeter.removeFromLeft(inputMeter.getWidth() * .5);
    meterL.setBounds(meterLSide);
    meterR.setBounds(inputMeter);

    auto outputMeter = bounds.removeFromRight(bounds.getWidth() * .2);
    auto outMeterLSide = outputMeter.removeFromLeft(outputMeter.getWidth() * .5);
    outMeterL.setBounds(outMeterLSide);
    outMeterR.setBounds(outputMeter);

    auto freqLeftBounds = bounds.removeFromLeft(bounds.getWidth() * .4);
    freqLeftBounds = freqLeftBounds.removeFromTop(bounds.getHeight() * .5);
    freqLeft.setBounds(freqLeftBounds);

    auto freqRightBounds = bounds.removeFromRight(bounds.getWidth() * .66);
    freqRightBounds = freqRightBounds.removeFromTop(bounds.getHeight() * .5);
    freqRight.setBounds(freqRightBounds);

    auto linkBounds = bounds.removeFromRight(bounds.getWidth());
    linkBounds = linkBounds.removeFromTop(bounds.getHeight() * .4);
    linkBounds = linkBounds.removeFromBottom(bounds.getHeight() * .2);
    link.setBounds(linkBounds);
    //link.setCentreRelative(.5f, .5f);

    bounds = getLocalBounds();
    bounds.removeFromLeft(bounds.getWidth() * .167);
    bounds.removeFromRight(bounds.getWidth() * .2);
    
    auto feedbackBounds = bounds.removeFromLeft(bounds.getWidth() * .5);
    feedbackBounds = feedbackBounds.removeFromBottom(bounds.getHeight() * .5);
    feedback.setBounds(feedbackBounds);

    auto dryWetBounds = bounds.removeFromLeft(bounds.getWidth());
    dryWetBounds = dryWetBounds.removeFromBottom(bounds.getHeight() * .5);
    dryWet.setBounds(dryWetBounds);

}

void SimpleDelayAudioProcessorEditor::timerCallback()
{
    //these get our rms level, and the set level function tells you how much of the rect you want
    meterL.setLevel(audioProcessor.getRMSValue(0));
    meterR.setLevel(audioProcessor.getRMSValue(1));

    meterL.repaint();
    meterR.repaint();

    outMeterL.setLevel(audioProcessor.getOutRMSValue(0));
    outMeterR.setLevel(audioProcessor.getOutRMSValue(1));

    outMeterL.repaint();
    outMeterR.repaint();
}

void SimpleDelayAudioProcessorEditor::ButtonLink::buttonClicked(juce::Button* button)
{

}

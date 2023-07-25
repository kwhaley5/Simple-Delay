/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

/*TODO;
* update value boxes
* Add Logo and stuff
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SimpleDelayAudioProcessorEditor::SimpleDelayAudioProcessorEditor(SimpleDelayAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p), freqLeftAT(audioProcessor.apvts, "freqLeft", freqLeft),
    freqRightAT(audioProcessor.apvts, "freqRight", freqRight), feedbackAT(audioProcessor.apvts, "feedback", feedback), 
    dryWetAT(audioProcessor.apvts, "dryWet", dryWet), linkAT(audioProcessor.apvts, "link", link), 
    wetAlgoAT(audioProcessor.apvts, "wetAlgo", wetAlgo)
{
    setLookAndFeel(&laf);
    
    addAndMakeVisible(meterL);
    addAndMakeVisible(meterR);
    addAndMakeVisible(outMeterL);
    addAndMakeVisible(outMeterR);

    freqLeft.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    freqLeft.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    addAndMakeVisible(freqLeft);

    freqRight.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    freqRight.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    addAndMakeVisible(freqRight);

    feedback.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedback.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    addAndMakeVisible(feedback);

    dryWet.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dryWet.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    addAndMakeVisible(dryWet);

    link.setToggleState(true, juce::dontSendNotification);
    link.setButtonText("LINK");
    addAndMakeVisible(link);

    wetAlgo.setToggleState(false, juce::dontSendNotification);
    wetAlgo.setButtonText("S&D");
    addAndMakeVisible(wetAlgo);

    wetAlgo.onStateChange = [this]()
    {
        if (!wetAlgo.getToggleState())
        {
            wetAlgo.setButtonText("S&D");
            wetAlgo.setTooltip("Dry/Wet Knob will increase the amount of the delayed signal only");
        }
        else
        {
            wetAlgo.setButtonText("DO");
            wetAlgo.setTooltip("Dry/Wet Knob will increase the amount of the delayed signal and proportinally reduce original signal");
        }
    };
    
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

    /*auto bounds = getLocalBounds();

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
    linkBounds = linkBounds.removeFromTop(bounds.getHeight() * .33);
    linkBounds = linkBounds.removeFromBottom(bounds.getHeight() * .2);
    link.setBounds(linkBounds);

    bounds = getLocalBounds();
    bounds.removeFromLeft(bounds.getWidth() * .167);
    bounds.removeFromRight(bounds.getWidth() * .2);

    auto feedbackBounds = bounds.removeFromLeft(bounds.getWidth() * .5);
    feedbackBounds = feedbackBounds.removeFromBottom(bounds.getHeight() * .5);
    feedback.setBounds(feedbackBounds);

    auto dryWetBounds = bounds.removeFromLeft(bounds.getWidth());
    dryWetBounds = dryWetBounds.removeFromBottom(bounds.getHeight() * .5);
    dryWet.setBounds(dryWetBounds);
    */
}

void SimpleDelayAudioProcessorEditor::resized()
{

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
    linkBounds = linkBounds.removeFromTop(bounds.getHeight() * .33);
    linkBounds = linkBounds.removeFromBottom(bounds.getHeight() * .2);
    link.setBounds(linkBounds);

    bounds = getLocalBounds();
    bounds.removeFromLeft(bounds.getWidth() * .167);
    bounds.removeFromRight(bounds.getWidth() * .2);
    
    auto feedbackBounds = bounds.removeFromLeft(bounds.getWidth() * .4);
    feedbackBounds = feedbackBounds.removeFromBottom(bounds.getHeight() * .5);
    feedback.setBounds(feedbackBounds);

    auto dryWetBounds = bounds.removeFromRight(bounds.getWidth() * .66);
    dryWetBounds = dryWetBounds.removeFromBottom(bounds.getHeight() * .5);
    dryWet.setBounds(dryWetBounds);

    auto wetAlgoBounds = bounds.removeFromRight(bounds.getWidth());
    wetAlgoBounds = wetAlgoBounds.removeFromBottom(bounds.getHeight() * .35);
    wetAlgoBounds = wetAlgoBounds.removeFromTop(bounds.getHeight() * .15);
    wetAlgo.setBounds(wetAlgoBounds);

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

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
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
    freqLeft.setName("Time");
    addAndMakeVisible(freqLeft);

    freqRight.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    freqRight.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    freqRight.setName("Time");
    addAndMakeVisible(freqRight);

    feedback.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    feedback.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    feedback.setName("Feedback");
    addAndMakeVisible(feedback);

    dryWet.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    dryWet.setTextBoxStyle(juce::Slider::NoTextBox, false, 100, 20);
    dryWet.setName("Dry/Wet");
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

    setSize (600, 450);
}

SimpleDelayAudioProcessorEditor::~SimpleDelayAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void SimpleDelayAudioProcessorEditor::paint (juce::Graphics& g)
{
    //Time to relearn flexbox....
    auto bounds = getLocalBounds();
    auto grad = juce::ColourGradient::ColourGradient(juce::Colour(186u, 34u, 34u), bounds.toFloat().getTopRight(), juce::Colour(186u, 34u, 34u), bounds.toFloat().getBottomLeft(), false);
    grad.addColour(.5f, juce::Colours::transparentBlack);
    g.setGradientFill(grad);

    g.fillAll ();

    auto inputMeter = bounds.removeFromLeft(bounds.getWidth() * .167);
    auto meterLSide = inputMeter.removeFromLeft(inputMeter.getWidth() * .5);
    meterL.setBounds(meterLSide);
    meterR.setBounds(inputMeter);

    auto outputMeter = bounds.removeFromRight(bounds.getWidth() * .2);
    auto outMeterLSide = outputMeter.removeFromLeft(outputMeter.getWidth() * .5);
    outMeterL.setBounds(outMeterLSide);
    outMeterR.setBounds(outputMeter);

    auto infoSpace = bounds.removeFromTop(bounds.getHeight() * .15);
    auto logoSpace = infoSpace.removeFromLeft(bounds.getWidth() * .41);
    auto textSpace = infoSpace.removeFromRight(bounds.getWidth() * .41);

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
    bounds.removeFromTop(bounds.getHeight() * .15);

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

    auto logo = juce::ImageCache::getFromMemory(BinaryData::KITIK_LOGO_NO_BKGD_png, BinaryData::KITIK_LOGO_NO_BKGD_pngSize);
    g.drawImage(logo, infoSpace.toFloat());

    auto newFont = juce::Font(juce::Typeface::createSystemTypefaceFor(BinaryData::OFFSHORE_TTF, BinaryData::OFFSHORE_TTFSize));
    g.setColour(juce::Colours::whitesmoke);
    g.setFont(newFont);
    g.setFont(30.f);
    g.drawFittedText("Simple", logoSpace, juce::Justification::centredRight, 1);
    g.drawFittedText("Delay", textSpace, juce::Justification::centredLeft, 1);
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

    auto infoSpace = bounds.removeFromTop(bounds.getHeight() * .15);
    auto logoSpace = infoSpace.removeFromLeft(bounds.getWidth() * .425);
    auto textSpace = infoSpace.removeFromRight(bounds.getWidth() * .425);

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
    bounds.removeFromTop(bounds.getHeight() * .15);
    
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

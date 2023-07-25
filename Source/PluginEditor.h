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
class SimpleDelayAudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    SimpleDelayAudioProcessorEditor (SimpleDelayAudioProcessor&);
    ~SimpleDelayAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback();

    struct Laf : juce::LookAndFeel_V4 {

        Laf() {

        }

        void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
            float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override
        {
            using namespace juce;

            auto unfill = Colour(15u, 15u, 15u);
            auto fill = Colour(64u, 194u, 230u);

            auto boundsFull = Rectangle<int>(x, y, width, height).toFloat();
            auto bounds = Rectangle<int>(x, y, width, height).toFloat().reduced(10);

            auto radius = jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
            auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
            auto lineW = jmin(8.0f, radius * 0.5f);
            auto arcRadius = radius - lineW * 0.5f;

            auto rootTwo = MathConstants<float>::sqrt2;

            Path backgroundArc;
            backgroundArc.addCentredArc(bounds.getCentreX(),
                bounds.getCentreY(),
                arcRadius,
                arcRadius,
                0.0f,
                rotaryStartAngle,
                rotaryEndAngle,
                true);

            g.setColour(unfill);
            g.strokePath(backgroundArc, PathStrokeType(lineW / 2, PathStrokeType::curved, PathStrokeType::rounded));

            if (slider.isEnabled())
            {
                Path valueArc;
                valueArc.addCentredArc(bounds.getCentreX(),
                    bounds.getCentreY(),
                    arcRadius,
                    arcRadius,
                    0.0f,
                    rotaryStartAngle,
                    toAngle,
                    true);

                g.setColour(fill);
                g.strokePath(valueArc, PathStrokeType(lineW / 2, PathStrokeType::curved, PathStrokeType::rounded));
            }

            //make circle with gradient
            float radialBlur = radius * 2.5;

            auto grad = ColourGradient::ColourGradient(Colour(186u, 34u, 34u), bounds.getCentreX(), bounds.getCentreY(), Colours::black, radialBlur, radialBlur, true);

            g.setGradientFill(grad);
            g.fillRoundedRectangle(boundsFull.getCentreX() - (radius * rootTwo / 2), boundsFull.getCentreY() - (radius * rootTwo / 2), radius * rootTwo, radius * rootTwo, radius * .7);

            //add circle around dial
            g.setColour(Colours::lightslategrey);
            g.drawRoundedRectangle(boundsFull.getCentreX() - (radius * rootTwo / 2), boundsFull.getCentreY() - (radius * rootTwo / 2), radius * rootTwo, radius * rootTwo, radius * .7, 1.5f);

            //make dial line
            g.setColour(Colours::whitesmoke);
            Point<float> thumbPoint(bounds.getCentreX() + radius / rootTwo * std::cos(toAngle - MathConstants<float>::halfPi), //This is one is farthest from center.
                bounds.getCentreY() + radius / rootTwo * std::sin(toAngle - MathConstants<float>::halfPi));

            Point<float> shortLine(bounds.getCentreX() + (arcRadius - (arcRadius / rootTwo)) * std::cos(toAngle - MathConstants<float>::halfPi), //This one is closer to the center
                bounds.getCentreY() + (arcRadius - (arcRadius / rootTwo)) * std::sin(toAngle - MathConstants<float>::halfPi));

            g.drawLine(shortLine.getX(), shortLine.getY(), thumbPoint.getX(), thumbPoint.getY(), lineW / 2);
            //g.drawLine(bounds.getCentreX(), bounds.getCentreY(), thumbPoint.getX(), thumbPoint.getY(), lineW / 2)

        }
        
    };

    struct ButtonLink : public juce::ToggleButton, public juce::Button::Listener
    {
        void buttonClicked(juce::Button* button) override;

    };

    struct LevelMeter : juce::Component
    {
        void paint(juce::Graphics& g) override
        {
            using namespace juce;

            //shapes the meters. May be a bit inefficeint, not sure the best way to move this stuff around, but it is there.
            auto bounds = getLocalBounds().toFloat();
            bounds = bounds.removeFromLeft(bounds.getWidth() * .75);
            bounds = bounds.removeFromRight(bounds.getWidth() * .66);
            bounds = bounds.removeFromTop(bounds.getHeight() * .9);
            bounds = bounds.removeFromBottom(bounds.getHeight() * .88);

            //get our base rectangle
            g.setColour(Colours::black);
            g.fillRoundedRectangle(bounds, 5.f);

            //get gradient
            auto gradient = ColourGradient::ColourGradient(Colours::green, bounds.getBottomLeft(), Colours::red, bounds.getTopLeft(), false);
            gradient.addColour(.5f, Colours::yellow);
            g.setGradientFill(gradient);

            //Show gradient
            auto levelMeterFill = jmap(level, -60.f, +6.f, 0.f, static_cast<float>(bounds.getHeight()));
            g.fillRoundedRectangle(bounds.removeFromBottom(levelMeterFill), 5.f);
        }

        //default value so the meters  are black when the plugin is launched
        void setLevel(float value) { level = value; }

    private:
        float level = -60.f;
    };

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SimpleDelayAudioProcessor& audioProcessor;

    Laf laf;
    LevelMeter meterR, meterL;
    LevelMeter outMeterR, outMeterL;

    juce::Slider freqLeft, freqRight, feedback, dryWet;
    juce::ToggleButton link;

    juce::AudioProcessorValueTreeState::SliderAttachment freqLeftAT, freqRightAT, feedbackAT, dryWetAT;
    juce::AudioProcessorValueTreeState::ButtonAttachment linkAT;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleDelayAudioProcessorEditor)
};

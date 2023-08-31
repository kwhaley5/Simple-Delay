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

            //Add text Values
            auto const fontSize = 15.f;

            auto str = String('none');
            auto value = slider.getValue();
            if (value <= 1) {
                value *= 100;
                str = String(value);
                str.append("%", 3);
            }
            else {
                str = String(value);
                str.append(" ms", 5);
            }

            auto strWidth = g.getCurrentFont().getStringWidth(str);

            Rectangle<float> r;
            r.setBottom(boundsFull.getBottom() - 10);
            r.setLeft(boundsFull.getCentre().getX() - strWidth);
            r.setRight(boundsFull.getCentre().getX() + strWidth);
            r.setTop(boundsFull.getBottom() - 30);

            g.setColour(Colours::whitesmoke);
            g.setFont(fontSize);
            g.drawFittedText(str, r.getX(), r.getY(), r.getWidth(), r.getHeight(), juce::Justification::centred, 1);

            auto name = slider.getName();
            strWidth = g.getCurrentFont().getStringWidth(name);
            r.setTop(0);
            r.setLeft(boundsFull.getCentre().getX() - strWidth);
            r.setRight(boundsFull.getCentre().getX() + strWidth);
            r.setBottom(20);
            g.setColour(Colours::whitesmoke);
            g.drawFittedText(name, r.getX(), r.getY(), r.getWidth(), r.getHeight(), juce::Justification::centred, 1);
            


        }

        void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
        {
            auto fontSize = juce::jmin(15.0f, (float)button.getHeight());
            auto getFont = g.getCurrentFont();
            auto rectWidth = getFont.getStringWidthFloat(button.getButtonText());

            juce::Rectangle<float> r;

            //Tweak this when you make your cutsom LNF headers and cpp files
            r.setSize(juce::jmax(20.f, rectWidth * 1.75f), fontSize * 1.2);
            r.translate((button.getWidth() / 2 - rectWidth*.85f), (button.getHeight() / 2 - fontSize / 2 -1.f));


            auto color = button.getToggleState() ? juce::Colour(64u, 194u, 230u) : juce::Colours::dimgrey;
            g.setColour(color);
            g.fillRoundedRectangle(r, fontSize/4);

            auto bounds = r.toFloat();
            g.setColour(juce::Colours::black);
            g.drawRoundedRectangle(bounds.getCentreX() - bounds.getWidth()/2, bounds.getCentreY() - bounds.getHeight()/2, bounds.getWidth(), bounds.getHeight(), fontSize / 4, 2);

            if (!button.isEnabled())
                g.setOpacity(0.5f);

            g.setColour(button.findColour(juce::ToggleButton::textColourId));
            g.setFont(fontSize);
            g.drawFittedText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, 10);
            
        }
        
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

    SimpleDelayAudioProcessor& audioProcessor;

    Laf laf;
    LevelMeter meterR, meterL;
    LevelMeter outMeterR, outMeterL;

    juce::Slider freqLeft, freqRight, feedback, dryWet;
    juce::ToggleButton link, wetAlgo;

    juce::AudioProcessorValueTreeState::SliderAttachment freqLeftAT, freqRightAT, feedbackAT, dryWetAT;
    juce::AudioProcessorValueTreeState::ButtonAttachment linkAT, wetAlgoAT;
    juce::TooltipWindow toggle {this, 1000};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleDelayAudioProcessorEditor)
};

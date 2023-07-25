/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Delay.h"

//==============================================================================
SimpleDelayAudioProcessor::SimpleDelayAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    freqLeft = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqLeft"));
    freqRight = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("freqRight"));
    feedback = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("feedback"));
    dryWet = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("dryWet"));
    link = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter("link"));
}

SimpleDelayAudioProcessor::~SimpleDelayAudioProcessor()
{
}

//==============================================================================
const juce::String SimpleDelayAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SimpleDelayAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SimpleDelayAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SimpleDelayAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SimpleDelayAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SimpleDelayAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SimpleDelayAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SimpleDelayAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SimpleDelayAudioProcessor::getProgramName (int index)
{
    return {};
}

void SimpleDelayAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SimpleDelayAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    //create the circular buffersize and assign it to the delaybuffer
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    for (auto& dl : delayLine)
    {
        dl.reset();
        dl.prepare(spec);
        dl.setMaximumDelayInSamples(sampleRate * 3);
    }

    for (auto& s : smoothedDelay)
    {
        s.reset(sampleRate, .05);
        s.setCurrentAndTargetValue(.5);
    }

    auto filterCoe = juce::dsp::IIR::Coefficients<float>::makeFirstOrderHighPass(sampleRate, 200);
    for (auto& f : filters)
    {
        f.prepare(spec);
        f.coefficients = filterCoe;
    }

}

void SimpleDelayAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SimpleDelayAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else

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

void SimpleDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    rmsLevelLeft = juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
    rmsLevelRight = juce::Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));

    //added to fix graphical bug, rms levels when no music was playing was below -60
    if (rmsLevelLeft < -60) {
        rmsLevelLeft = -60;
    }
    if (rmsLevelRight < -60) {
        rmsLevelRight = -60;
    }

    
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for (auto channel = 0; channel < buffer.getNumChannels(); ++channel)
        createDelay(channel, delayLine[channel], buffer);


    rmsOutLevelLeft = juce::Decibels::gainToDecibels(buffer.getRMSLevel(0, 0, buffer.getNumSamples()));
    rmsOutLevelRight = juce::Decibels::gainToDecibels(buffer.getRMSLevel(1, 0, buffer.getNumSamples()));

    //added to fix graphical bug, rms levels when no music was playing was below -60
    if (rmsOutLevelLeft < -60) {
        rmsOutLevelLeft = -60;
    }
    if (rmsOutLevelRight < -60) {
        rmsOutLevelRight = -60;
    }

}

void SimpleDelayAudioProcessor::createDelay(int channel, juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> &delayLine, juce::AudioBuffer<float>& buffer)
{
    auto delayTime = 0;

    if (channel == 0) {
        smoothedDelay[channel].setTargetValue((freqLeft->get() / 1000));
    }
    else {
        smoothedDelay[channel].setTargetValue((freqRight->get() / 1000));
    }

    auto& filter = filters[channel];

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    auto& inputBlock = context.getInputBlock();
    auto& ouputBlock = context.getOutputBlock();
    auto numSamples = ouputBlock.getNumSamples();

    auto* input = inputBlock.getChannelPointer(channel);
    auto* output = ouputBlock.getChannelPointer(channel);
   

    for (int i = 0; i < numSamples; i++)
    {
        auto nextDelayTime = smoothedDelay[channel].getNextValue() * getSampleRate();
        auto delayedSample = filter.processSample(delayLine.popSample(channel, nextDelayTime));
        auto inDelay = std::tanh(input[i] + feedback->get() * delayedSample);
        delayLine.pushSample(channel, inDelay);
        output[i] = (input[i] * (1 - dryWet->get())) + (delayedSample * dryWet->get());
        //output[i] = std::tanh(input[i] + dryWet->get() * delayedSample);
    }
}

//==============================================================================
bool SimpleDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleDelayAudioProcessor::createEditor()
{
    return new SimpleDelayAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void SimpleDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void SimpleDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleDelayAudioProcessor::createParameterLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    auto freqRange = NormalisableRange<float>(1, 3000, 1, 1);
    auto feedbackRange = NormalisableRange<float>(.01, 1, .01, 1);
    auto dryWetRange = NormalisableRange<float>(.01, 1, .01, 1);

    layout.add(std::make_unique<AudioParameterFloat>("freqLeft", "Frequency Left", freqRange, 50));
    layout.add(std::make_unique<AudioParameterFloat>("freqRight", "Frequency Right", freqRange, 50));
    layout.add(std::make_unique<AudioParameterFloat>("feedback", "Feedback", feedbackRange, .5));
    layout.add(std::make_unique<AudioParameterFloat>("dryWet", "Dry/Wet", dryWetRange, .5));
    layout.add(std::make_unique<AudioParameterBool>("link", "Link", true));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleDelayAudioProcessor();
}

float SimpleDelayAudioProcessor::getRMSValue(int channel) //get function to provide the rms levels for the editor
{
    jassert(channel == 0 || channel == 1);
    if (channel == 0) {
        return rmsLevelLeft;
    }
    else if (channel == 1) {
        return rmsLevelRight;
    }
    else {
        return 0.0f;
    }
}

float SimpleDelayAudioProcessor::getOutRMSValue(int channel) //same as above
{
    jassert(channel == 0 || channel == 1);
    if (channel == 0) {
        return rmsOutLevelLeft;
    }
    else if (channel == 1) {
        return rmsOutLevelRight;
    }
    else {
        return 0.0f;
    }
}

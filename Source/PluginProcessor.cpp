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
    auto bufferSize = sampleRate * 3.0;
    delayBuffer.setSize(getNumOutputChannels(), static_cast<int>(bufferSize));
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

void SimpleDelayAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    freq = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter("Frequency"));

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

    
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        if (delayBuffer.getNumSamples() > buffer.getNumSamples() + bufferIndex)
        {
            delayBuffer.copyFrom(channel, bufferIndex, channelData, buffer.getNumSamples());
        }
        else 
        {
            auto samplesLeft = delayBuffer.getNumSamples() - bufferIndex;
            delayBuffer.copyFrom(channel, bufferIndex, channelData, samplesLeft);
            auto remainingSamples = buffer.getNumSamples() - samplesLeft;
            delayBuffer.copyFrom(channel, 0, channelData + samplesLeft, remainingSamples); //add the plus samples Left to make sure you get the remaining samples. 

            auto readPosition = bufferIndex - (ceil(getSampleRate() * *freq / 1000));

            if (readPosition < 0) //makes sure we loop back around
            {
                readPosition += delayBuffer.getNumSamples();
            }

            //NEXT STEP: do the same above, to loop back around the circular buffer
        }
    }

    bufferIndex += buffer.getNumSamples();
    bufferIndex %= delayBuffer.getNumSamples(); //if delay buffer > then buffer index, the remainder is just the original index, but if index is bigger, it will allow it to wrap around

    //will probably change this because that doesn't make as much sense as is
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

//==============================================================================
bool SimpleDelayAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SimpleDelayAudioProcessor::createEditor()
{
    return new SimpleDelayAudioProcessorEditor (*this);
}

//==============================================================================
void SimpleDelayAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SimpleDelayAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

juce::AudioProcessorValueTreeState::ParameterLayout SimpleDelayAudioProcessor::createParameterLayout()
{
    using namespace juce;

    AudioProcessorValueTreeState::ParameterLayout layout;

    auto freqRange = NormalisableRange<float>(1, 3000, 1, 1);
    auto feedbackRange = NormalisableRange<float>(.01, 1, .01, 1);
    auto dryWetRange = NormalisableRange<float>(.01, 1, .01, 1);

    layout.add(std::make_unique<AudioParameterFloat>("freq", "Frequency", freqRange, 50));
    layout.add(std::make_unique<AudioParameterFloat>("feedback", "Feedback", feedbackRange, .5));
    layout.add(std::make_unique<AudioParameterFloat>("dryWet", "Dry/Wet", dryWetRange, .5));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SimpleDelayAudioProcessor();
}

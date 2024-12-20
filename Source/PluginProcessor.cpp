/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HiJUCEAudioProcessor::HiJUCEAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), treeState(*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
    treeState.addParameterListener("gain", this);
    treeState.addParameterListener("phase", this);
    treeState.addParameterListener("choice", this);
}

HiJUCEAudioProcessor::~HiJUCEAudioProcessor()
{
    treeState.removeParameterListener("gain", this);
    treeState.removeParameterListener("phase", this);
    treeState.removeParameterListener("choice", this);
}

juce::AudioProcessorValueTreeState::ParameterLayout HiJUCEAudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr<juce::RangedAudioParameter>> params;
    
    juce::StringArray choices = {"Compressor", "EQ", "Reverb"};
    
    auto pGain = std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{"gain", 1}, "Gain", -24.0, 24.0, 0.0);
    auto pPhase = std::make_unique<juce::AudioParameterBool>(juce::ParameterID{"phase"}, "Phase", false);
    auto pChoice = std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{"choice"}, "Choice", choices, 0);
    
    params.push_back(std::move(pGain));
    params.push_back(std::move(pPhase));
    params.push_back(std::move(pChoice));
    
    return { params.begin(), params.end() };
}

void HiJUCEAudioProcessor::parameterChanged(const juce::String &parameterID, float newValue)
{
    if(parameterID == "gain") {
        rawGain = juce::Decibels::decibelsToGain(newValue);
        DBG("Gain is: " << newValue);
    }
    if(parameterID == "phase") {
        phase = newValue;
        DBG("Phase is: " << newValue);
    }
}

//==============================================================================
const juce::String HiJUCEAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HiJUCEAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool HiJUCEAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool HiJUCEAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double HiJUCEAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HiJUCEAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int HiJUCEAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HiJUCEAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String HiJUCEAudioProcessor::getProgramName (int index)
{
    return {};
}

void HiJUCEAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void HiJUCEAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    phase = *treeState.getRawParameterValue("phase");
    rawGain = juce::Decibels::decibelsToGain(static_cast<float>(*treeState.getRawParameterValue("gain")));
}

void HiJUCEAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HiJUCEAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void HiJUCEAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    // DSP Block
    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            if(phase) {
                channelData[sample] *= rawGain * -1.0;
            } else {
                channelData[sample] *= rawGain;
            }
        }
    }
}

//==============================================================================
bool HiJUCEAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* HiJUCEAudioProcessor::createEditor()
{
    return new HiJUCEAudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void HiJUCEAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream stream(destData, false);
    treeState.state.writeToStream(stream);
}

void HiJUCEAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData (data, size_t(sizeInBytes));
    
    if(tree.isValid()) {
        treeState.state = tree;
        phase = *treeState.getRawParameterValue("phase");
        rawGain = juce::Decibels::decibelsToGain(static_cast<float>(*treeState.getRawParameterValue("gain")));
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HiJUCEAudioProcessor();
}

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
TabVSTAudioProcessor::TabVSTAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       )
{
}

TabVSTAudioProcessor::~TabVSTAudioProcessor()
{
}

//==============================================================================
const juce::String TabVSTAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool TabVSTAudioProcessor::acceptsMidi() const
{
    return false;
}

bool TabVSTAudioProcessor::producesMidi() const
{
    return false;
}

bool TabVSTAudioProcessor::isMidiEffect() const
{
    return false;
}

double TabVSTAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int TabVSTAudioProcessor::getNumPrograms()
{
    return 1;
}

int TabVSTAudioProcessor::getCurrentProgram()
{
    return 0;
}

void TabVSTAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String TabVSTAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void TabVSTAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void TabVSTAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused (sampleRate, samplesPerBlock);
}

void TabVSTAudioProcessor::releaseResources()
{
}

bool TabVSTAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // We support any layout since we're just passing audio through
    return true;
}

void TabVSTAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                         juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;

    // This plugin doesn't process audio, it just passes it through
    // The guitar tabs are purely visual/data storage
}

//==============================================================================
bool TabVSTAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* TabVSTAudioProcessor::createEditor()
{
    return new TabVSTAudioProcessorEditor (*this);
}

//==============================================================================
void TabVSTAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Save the tab data so it persists with the DAW project
    auto xml = tabEngine.saveToXML();
    copyXmlToBinary (*xml, destData);
}

void TabVSTAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restore the tab data when loading a DAW project
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr)
        tabEngine.loadFromXML (*xmlState);
}

//==============================================================================
// This creates new instances of the plugin
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new TabVSTAudioProcessor();
}

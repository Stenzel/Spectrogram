/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SpectrogramAudioProcessor::SpectrogramAudioProcessor() : fft(10), window(1024,juce::dsp::WindowingFunction<float>::WindowingMethod::hann)
#ifndef JucePlugin_PreferredChannelConfigurations
     , AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
 
{
    gls = 0;
    fftpos = 0;
    memset(fftbuf,0,sizeof(fftbuf));
}

SpectrogramAudioProcessor::~SpectrogramAudioProcessor()
{
    gls = 0;
}

//==============================================================================
const juce::String SpectrogramAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SpectrogramAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SpectrogramAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SpectrogramAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SpectrogramAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SpectrogramAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SpectrogramAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SpectrogramAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SpectrogramAudioProcessor::getProgramName (int index)
{
    return {};
}

void SpectrogramAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SpectrogramAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void SpectrogramAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SpectrogramAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void SpectrogramAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    if(0 == totalNumInputChannels || !gls) return;
    
    int nsmp = buffer.getNumSamples();
    const float *src = buffer.getReadPointer(0);
    int required = 1024 - fftpos;
    int n = nsmp;
    if(n > required) n = required;
    if(n)
    {
        memcpy(fftbuf + fftpos,src,n * sizeof(float));
        fftpos += n;
    }
    
    if(fftpos >= 1024)
    {
        window.multiplyWithWindowingTable(fftbuf,1024);
        fft.performFrequencyOnlyForwardTransform(fftbuf,true);
        gls->newline(fftbuf);

        fftpos = 0;
        
        src += n;
        nsmp -= n;
        n = nsmp;
        if(n > 1024) n = 1024;
        if(n)
        {
            memcpy(fftbuf,src,n * sizeof(float));
            fftpos = n;
        }
    }

    //silence
    for (int i = 0; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

}

//==============================================================================
bool SpectrogramAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SpectrogramAudioProcessor::createEditor()
{
    return new SpectrogramAudioProcessorEditor (*this);
}

//==============================================================================
void SpectrogramAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SpectrogramAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SpectrogramAudioProcessor();
}

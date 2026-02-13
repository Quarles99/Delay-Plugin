/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayPluginAudioProcessor::DelayPluginAudioProcessor()
     : AudioProcessor (BusesProperties()

                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
		                apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

DelayPluginAudioProcessor::~DelayPluginAudioProcessor()
{
}

//==============================================================================
const juce::String DelayPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DelayPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DelayPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DelayPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DelayPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DelayPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DelayPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DelayPluginAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DelayPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void DelayPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DelayPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{

	currentSampleRate = sampleRate;

	float maxDelayTimeMs = 2000.0f; // 2 seconds max delay time

	int maxDelaySamples = (int)(currentSampleRate * (maxDelayTimeMs / 1000.0f));

	delayBufferSize = maxDelaySamples + samplesPerBlock;

	//Make the delay buffer bigger than delay time to avoid buffer overrun
	delayBufferSize = maxDelaySamples + samplesPerBlock;

	delayBuffer.setSize(getTotalNumInputChannels(), delayBufferSize);
	delayBuffer.clear();

	writePosition = 0;

	smoothedDelaySamples.reset(currentSampleRate, 0.05); // 50 ms smoothing time

}

void DelayPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
	delayBuffer.clear();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DelayPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void DelayPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    
	float delayTimeMs = apvts.getRawParameterValue(paramDelayTime)->load();
	float feedback = apvts.getRawParameterValue(paramFeedback)->load();
	float mix = apvts.getRawParameterValue(paramMix)->load();

    //Smoothing
	float targetDelaySamples = (currentSampleRate * (delayTimeMs / 1000.0f));
	smoothedDelaySamples.setTargetValue(targetDelaySamples);

    int numChannels = buffer.getNumChannels();
	int numSamples = buffer.getNumSamples();

	float dryMix = 1.0f - mix;
	float wetMix = mix;

	for (int channel = 0; channel < numChannels; ++channel)
    {

        float* channelData = buffer.getWritePointer(channel);
        float* delayData = delayBuffer.getWritePointer(channel);

		int localWritePosition = writePosition;

            for (int i = 0; i < numSamples; ++i)
            {
				float delaySamples = smoothedDelaySamples.getNextValue();

				float readPosition = (float)localWritePosition - delaySamples;
                
                while (readPosition < 0.0f)
                {
					readPosition += delayBufferSize;
                }

				int index1 = (int)readPosition;
                int index2 = index1 + 1;

                if (index2 >= delayBufferSize)
                    index2 = 0;

				float fraction = readPosition - (float)index1;

				float delayedSample = delayData[index1] + fraction * (delayData[index2] - delayData[index1]);

                float inputSample = channelData[i];

				float outputSample = (inputSample * dryMix) + (delayedSample * wetMix);

				channelData[i] = outputSample;

                //Feedback writeback
				delayData[localWritePosition] = inputSample + (delayedSample * feedback);

				localWritePosition++;

				if (localWritePosition >= delayBufferSize)
                    localWritePosition = 0;
            }
    }

	writePosition += numSamples;
	writePosition %= delayBufferSize;
}

//==============================================================================
bool DelayPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DelayPluginAudioProcessor::createEditor()
{
    return new DelayPluginAudioProcessorEditor (*this);
}

//==============================================================================
void DelayPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DelayPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DelayPluginAudioProcessor();
}

//Create parameter layout for the plugin
juce::AudioProcessorValueTreeState::ParameterLayout
DelayPluginAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "delayTime",
        "Delay Time",
        juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f), 500.0f
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "feedback",
        "Feedback",
        juce::NormalisableRange<float>(0.0f, 0.95f, 0.01f), 0.4f
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "mix",
        "Mix",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f
    ));

    return layout;
}

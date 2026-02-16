/*
  ==============================================================================

	PluginProcessor.cpp

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CooeeAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"time", "Time (ms)",
		juce::NormalisableRange<float>(1.0f, 2000.0f, 0.0f, 0.5f),
		400.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"feedback", "Feedback",
		juce::NormalisableRange<float>(0.0f, 0.95f),
		0.35f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"mix", "Mix",
		juce::NormalisableRange<float>(0.0f, 1.0f),
		0.35f));

	return layout;
}

//==============================================================================
CooeeAudioProcessor::CooeeAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	)
#else
	: AudioProcessor()
#endif
	, parameters(*this, nullptr, juce::Identifier("CooeeParams"), createParameterLayout())
{
}

CooeeAudioProcessor::~CooeeAudioProcessor()
{
}

//==============================================================================
const juce::String CooeeAudioProcessor::getName() const
{
	return JucePlugin_Name;
}

bool CooeeAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
	return true;
#else
	return false;
#endif
}

bool CooeeAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
	return true;
#else
	return false;
#endif
}

bool CooeeAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
	return true;
#else
	return false;
#endif
}

double CooeeAudioProcessor::getTailLengthSeconds() const
{
	return 0.0;
}

int CooeeAudioProcessor::getNumPrograms()
{
	return 1;

}

int CooeeAudioProcessor::getCurrentProgram()
{
	return 0;
}

void CooeeAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String CooeeAudioProcessor::getProgramName(int index)
{
	return {};
}

void CooeeAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

//==============================================================================
void CooeeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	maxDelaySamples = static_cast<int>(sampleRate * 2.0);

	delayBuffer.clear();
	delayBuffer.resize(getTotalNumOutputChannels());

	for (int ch = 0; ch < getTotalNumOutputChannels(); ++ch)
		delayBuffer[ch].assign(maxDelaySamples, 0.0f);

	writePosition = 0;
}

void CooeeAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CooeeAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
	juce::ignoreUnused(layouts);
	return true;
#else
	if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
		&& layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
		return false;

#if ! JucePlugin_IsSynth
	if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
		return false;
#endif

	return true;
#endif
}
#endif

void CooeeAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	juce::ignoreUnused(midiMessages);
	juce::ScopedNoDenormals noDenormals;

	auto totalNumInputChannels = getTotalNumInputChannels();
	auto totalNumOutputChannels = getTotalNumOutputChannels();

	for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	const float timeMs = *parameters.getRawParameterValue("time");
	const float feedback = *parameters.getRawParameterValue("feedback");
	const float mix = *parameters.getRawParameterValue("mix");

	const int numSamples = buffer.getNumSamples();

	int delaySamples = static_cast<int>((timeMs / 1000.0f) * getSampleRate());
	delaySamples = juce::jlimit(1, maxDelaySamples - 1, delaySamples);

	const int startWrite = writePosition;

	for (int ch = 0; ch < totalNumInputChannels; ++ch)
	{
		auto* channelData = buffer.getWritePointer(ch);
		auto& d = delayBuffer[ch];

		int write = startWrite;

		for (int n = 0; n < numSamples; ++n)
		{
			int readPos = write - delaySamples;
			if (readPos < 0) readPos += maxDelaySamples;

			const float delayed = d[readPos];
			const float in = channelData[n];

			d[write] = in + delayed * feedback;
			channelData[n] = in * (1.0f - mix) + delayed * mix;

			++write;
			if (write >= maxDelaySamples)
				write = 0;
		}
	}
	writePosition += numSamples;
	writePosition %= maxDelaySamples;
}

//==============================================================================
bool CooeeAudioProcessor::hasEditor() const
{
	return true;
}

juce::AudioProcessorEditor* CooeeAudioProcessor::createEditor()
{
	return new CooeeAudioProcessorEditor(*this);
}

//==============================================================================
void CooeeAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
}

void CooeeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new CooeeAudioProcessor();
}

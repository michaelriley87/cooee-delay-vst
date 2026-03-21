/*
  ==============================================================================

	PluginProcessor.cpp

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout CooeeAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"time", "Time (ms)",
		juce::NormalisableRange<float>(1.0f, 2000.0f, 1.0f),
		400.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"feedback", "Feedback",
		juce::NormalisableRange<float>(0.0f, 95.0f, 1.0f),
		35.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"mix", "Mix",
		juce::NormalisableRange<float>(0.0f, 100.0f, 1.0f),
		35.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"lowCut", "Low Cut",
		juce::NormalisableRange<float>(20.0f, 2000.0f, 1.0f, 0.5f),
		20.0f));

	layout.add(std::make_unique<juce::AudioParameterFloat>(
		"highCut", "High Cut",
		juce::NormalisableRange<float>(1000.0f, 20000.0f, 1.0f, 0.5f),
		20000.0f));

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
	juce::ignoreUnused(index);
}

const juce::String CooeeAudioProcessor::getProgramName(int index)
{
	juce::ignoreUnused(index);
	return {};
}

void CooeeAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
	juce::ignoreUnused(index, newName);
}

//==============================================================================
void CooeeAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	juce::ignoreUnused(samplesPerBlock);

	currentSampleRate = sampleRate;
	const int totalNumChannels = getTotalNumOutputChannels();

	lowCutFilters.clear();
	highCutFilters.clear();

	lowCutFilters.resize(totalNumChannels);
	highCutFilters.resize(totalNumChannels);

	juce::dsp::ProcessSpec spec;
	spec.sampleRate = sampleRate;
	spec.maximumBlockSize = (juce::uint32)samplesPerBlock;
	spec.numChannels = 1;

	for (int ch = 0; ch < totalNumChannels; ++ch)
	{
		lowCutFilters[ch].reset();
		highCutFilters[ch].reset();

		lowCutFilters[ch].setType(juce::dsp::StateVariableTPTFilterType::highpass);
		highCutFilters[ch].setType(juce::dsp::StateVariableTPTFilterType::lowpass);

		lowCutFilters[ch].prepare(spec);
		highCutFilters[ch].prepare(spec);
	}

	maxDelaySamples = (int)(sampleRate * 2.0);

	delayBuffer.clear();
	delayBuffer.resize(totalNumChannels);

	for (int ch = 0; ch < totalNumChannels; ++ch)
		delayBuffer[ch].assign(maxDelaySamples, 0.0f);

	writePosition = 0;

	const double rampTime = 0.02;

	smoothedTime.reset(sampleRate, rampTime);
	smoothedFeedback.reset(sampleRate, rampTime);
	smoothedMix.reset(sampleRate, rampTime);
	smoothedLowCut.reset(sampleRate, rampTime);
	smoothedHighCut.reset(sampleRate, rampTime);

	smoothedTime.setCurrentAndTargetValue(*parameters.getRawParameterValue("time"));
	smoothedFeedback.setCurrentAndTargetValue(*parameters.getRawParameterValue("feedback"));
	smoothedMix.setCurrentAndTargetValue(*parameters.getRawParameterValue("mix"));
	smoothedLowCut.setCurrentAndTargetValue(*parameters.getRawParameterValue("lowCut"));
	smoothedHighCut.setCurrentAndTargetValue(*parameters.getRawParameterValue("highCut"));
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

	const int totalNumInputChannels = getTotalNumInputChannels();
	const int totalNumOutputChannels = getTotalNumOutputChannels();
	const int numSamples = buffer.getNumSamples();

	for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
		buffer.clear(i, 0, numSamples);

	smoothedTime.setTargetValue(*parameters.getRawParameterValue("time"));
	smoothedFeedback.setTargetValue(*parameters.getRawParameterValue("feedback"));
	smoothedMix.setTargetValue(*parameters.getRawParameterValue("mix"));
	smoothedLowCut.setTargetValue(*parameters.getRawParameterValue("lowCut"));
	smoothedHighCut.setTargetValue(*parameters.getRawParameterValue("highCut"));

	const int startWrite = writePosition;

	for (int ch = 0; ch < totalNumInputChannels; ++ch)
	{
		auto* channelData = buffer.getWritePointer(ch);
		auto& d = delayBuffer[ch];

		int write = startWrite;

		for (int n = 0; n < numSamples; ++n)
		{
			float timeMs = smoothedTime.getNextValue();
			float feedback = smoothedFeedback.getNextValue() * 0.01f;
			float mix = smoothedMix.getNextValue() * 0.01f;
			float lowCutHz = smoothedLowCut.getNextValue();
			float highCutHz = smoothedHighCut.getNextValue();

			const float nyquist = (float)(currentSampleRate * 0.5);

			highCutHz = juce::jlimit(1000.0f, nyquist - 100.0f, highCutHz);
			lowCutHz = juce::jlimit(20.0f, highCutHz - 50.0f, lowCutHz);

			int delaySamples = (int)((timeMs / 1000.0f) * getSampleRate());
			delaySamples = juce::jlimit(1, maxDelaySamples - 1, delaySamples);

			int readPos = write - delaySamples;
			if (readPos < 0)
				readPos += maxDelaySamples;

			const float delayed = d[readPos];
			const float in = channelData[n];

			lowCutFilters[ch].setCutoffFrequency(lowCutHz);
			highCutFilters[ch].setCutoffFrequency(highCutHz);

			float feedbackSignal = delayed * feedback;
			feedbackSignal = lowCutFilters[ch].processSample(0, feedbackSignal);
			feedbackSignal = highCutFilters[ch].processSample(0, feedbackSignal);

			if (!std::isfinite(feedbackSignal))
				feedbackSignal = 0.0f;

			const float writeSample = juce::jlimit(-1.0f, 1.0f, in + feedbackSignal);
			d[write] = std::isfinite(writeSample) ? writeSample : 0.0f;

			const float output = in * (1.0f - mix) + delayed * mix;
			channelData[n] = std::isfinite(output) ? output : 0.0f;

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
	auto state = parameters.copyState();
	std::unique_ptr<juce::XmlElement> xml(state.createXml());
	copyXmlToBinary(*xml, destData);
}

void CooeeAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
	std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

	if (xml && xml->hasTagName(parameters.state.getType()))
	{
		parameters.replaceState(juce::ValueTree::fromXml(*xml));
	}
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
	return new CooeeAudioProcessor();
}
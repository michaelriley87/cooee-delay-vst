/*
  ==============================================================================

	PluginEditor.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CooeeAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
	explicit CooeeAudioProcessorEditor(CooeeAudioProcessor&);
	~CooeeAudioProcessorEditor() override;

	void paint(juce::Graphics&) override;
	void resized() override;

private:
	CooeeAudioProcessor& audioProcessor;

	juce::Slider timeSlider;
	juce::Slider feedbackSlider;
	juce::Slider mixSlider;
	juce::Slider lowCutSlider;
	juce::Slider highCutSlider;

	juce::Label timeLabel;
	juce::Label feedbackLabel;
	juce::Label mixLabel;
	juce::Label lowCutLabel;
	juce::Label highCutLabel;

	using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
	std::unique_ptr<Attachment> timeAttachment;
	std::unique_ptr<Attachment> feedbackAttachment;
	std::unique_ptr<Attachment> mixAttachment;
	std::unique_ptr<Attachment> lowCutAttachment;
	std::unique_ptr<Attachment> highCutAttachment;

	static void configureKnob(juce::Slider& s);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CooeeAudioProcessorEditor)
};
/*
  ==============================================================================

	PluginEditor.h

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CooeeAudioProcessorEditor : public juce::AudioProcessorEditor,
	private juce::Timer
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

	juce::TextButton syncButton;
	juce::ComboBox divisionBox;
	juce::Label syncLabel;

	using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
	using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
	using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

	std::unique_ptr<SliderAttachment> timeAttachment;
	std::unique_ptr<SliderAttachment> feedbackAttachment;
	std::unique_ptr<SliderAttachment> mixAttachment;
	std::unique_ptr<SliderAttachment> lowCutAttachment;
	std::unique_ptr<SliderAttachment> highCutAttachment;
	std::unique_ptr<ButtonAttachment> syncAttachment;
	std::unique_ptr<ComboBoxAttachment> divisionAttachment;

	static void configureKnob(juce::Slider& s);
	void updateSyncUiState();
	void timerCallback() override;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CooeeAudioProcessorEditor)
};
/*
  ==============================================================================

	PluginEditor.cpp

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static void configureLabel(juce::Label& l, const juce::String& text)
{
	l.setText(text, juce::dontSendNotification);
	l.setJustificationType(juce::Justification::centred);
}

void CooeeAudioProcessorEditor::configureKnob(juce::Slider& s)
{
	s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
	s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
}

CooeeAudioProcessorEditor::CooeeAudioProcessorEditor(CooeeAudioProcessor& p)
	: AudioProcessorEditor(&p)
	, audioProcessor(p)
{
	configureKnob(timeSlider);
	configureKnob(feedbackSlider);
	configureKnob(mixSlider);
	configureKnob(lowCutSlider);
	configureKnob(highCutSlider);

	configureLabel(timeLabel, "Time (ms)");
	configureLabel(feedbackLabel, "Feedback");
	configureLabel(mixLabel, "Mix");
	configureLabel(lowCutLabel, "Low Cut");
	configureLabel(highCutLabel, "High Cut");

	addAndMakeVisible(timeSlider);
	addAndMakeVisible(feedbackSlider);
	addAndMakeVisible(mixSlider);
	addAndMakeVisible(lowCutSlider);
	addAndMakeVisible(highCutSlider);

	addAndMakeVisible(timeLabel);
	addAndMakeVisible(feedbackLabel);
	addAndMakeVisible(mixLabel);
	addAndMakeVisible(lowCutLabel);
	addAndMakeVisible(highCutLabel);

	timeAttachment = std::make_unique<Attachment>(audioProcessor.parameters, "time", timeSlider);
	feedbackAttachment = std::make_unique<Attachment>(audioProcessor.parameters, "feedback", feedbackSlider);
	mixAttachment = std::make_unique<Attachment>(audioProcessor.parameters, "mix", mixSlider);
	lowCutAttachment = std::make_unique<Attachment>(audioProcessor.parameters, "lowCut", lowCutSlider);
	highCutAttachment = std::make_unique<Attachment>(audioProcessor.parameters, "highCut", highCutSlider);

	setSize(700, 180);
}

void CooeeAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::black);
}

void CooeeAudioProcessorEditor::resized()
{
	auto area = getLocalBounds().reduced(20);

	const int colW = area.getWidth() / 5;

	auto c1 = area.removeFromLeft(colW);
	auto c2 = area.removeFromLeft(colW);
	auto c3 = area.removeFromLeft(colW);
	auto c4 = area.removeFromLeft(colW);
	auto c5 = area;

	const int labelH = 20;
	const int knobH = 120;

	timeLabel.setBounds(c1.removeFromTop(labelH));
	timeSlider.setBounds(c1.removeFromTop(knobH));

	feedbackLabel.setBounds(c2.removeFromTop(labelH));
	feedbackSlider.setBounds(c2.removeFromTop(knobH));

	mixLabel.setBounds(c3.removeFromTop(labelH));
	mixSlider.setBounds(c3.removeFromTop(knobH));

	lowCutLabel.setBounds(c4.removeFromTop(labelH));
	lowCutSlider.setBounds(c4.removeFromTop(knobH));

	highCutLabel.setBounds(c5.removeFromTop(labelH));
	highCutSlider.setBounds(c5.removeFromTop(knobH));
}

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

	configureLabel(timeLabel, "Time (ms)");
	configureLabel(feedbackLabel, "Feedback");
	configureLabel(mixLabel, "Mix");

	addAndMakeVisible(timeSlider);
	addAndMakeVisible(feedbackSlider);
	addAndMakeVisible(mixSlider);

	addAndMakeVisible(timeLabel);
	addAndMakeVisible(feedbackLabel);
	addAndMakeVisible(mixLabel);

	timeAttachment = std::make_unique<Attachment>(audioProcessor.parameters, "time", timeSlider);
	feedbackAttachment = std::make_unique<Attachment>(audioProcessor.parameters, "feedback", feedbackSlider);
	mixAttachment = std::make_unique<Attachment>(audioProcessor.parameters, "mix", mixSlider);

	setSize(420, 180);
}

void CooeeAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colours::black);
}

void CooeeAudioProcessorEditor::resized()
{
	auto area = getLocalBounds().reduced(20);

	const int colW = area.getWidth() / 3;

	auto c1 = area.removeFromLeft(colW);
	auto c2 = area.removeFromLeft(colW);
	auto c3 = area;

	const int labelH = 20;
	const int knobH = 120;

	timeLabel.setBounds(c1.removeFromTop(labelH));
	timeSlider.setBounds(c1.removeFromTop(knobH));

	feedbackLabel.setBounds(c2.removeFromTop(labelH));
	feedbackSlider.setBounds(c2.removeFromTop(knobH));

	mixLabel.setBounds(c3.removeFromTop(labelH));
	mixSlider.setBounds(c3.removeFromTop(knobH));
}

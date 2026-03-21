/*
  ==============================================================================

	PluginEditor.cpp

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::Font makeMontserratFont(float height, bool bold)
{
	static auto typeface = juce::Typeface::createSystemTypefaceFor(
		BinaryData::MontserratVariableFont_wght_ttf,
		BinaryData::MontserratVariableFont_wght_ttfSize
	);

	juce::Font font(typeface);
	font.setHeight(height);
	font.setBold(bold);
	return font;
}

static juce::Font makeBebasFont(float height)
{
	static auto typeface = juce::Typeface::createSystemTypefaceFor(
		BinaryData::BebasNeueRegular_ttf,
		BinaryData::BebasNeueRegular_ttfSize
	);

	juce::Font font(typeface);
	font.setHeight(height);
	return font;
}

static void configureLabel(juce::Label& l, const juce::String& text)
{
	l.setText(text, juce::dontSendNotification);
	l.setJustificationType(juce::Justification::centred);
	l.setColour(juce::Label::textColourId, juce::Colours::white);
	l.setFont(makeMontserratFont(16.0f, true));
}

class FlatKnobLook : public juce::LookAndFeel_V4
{
public:
	void drawRotarySlider(juce::Graphics& g,
		int x, int y, int width, int height,
		float sliderPos,
		float rotaryStartAngle,
		float rotaryEndAngle,
		juce::Slider&) override
	{
		float size = juce::jmin((float)width, (float)height);

		juce::Rectangle<float> bounds(
			(float)x + ((float)width - size) * 0.5f,
			(float)y + ((float)height - size) * 0.5f,
			size,
			size
		);

		float radius = size * 0.5f;
		float centerX = bounds.getCentreX();
		float centerY = bounds.getCentreY();

		float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

		g.setColour(juce::Colour(0xff004d25));
		g.fillEllipse(bounds);

		juce::Path indicator;

		float indicatorLength = radius;
		float thickness = radius * 0.18f;

		indicator.addRectangle(
			-thickness * 0.5f,
			-indicatorLength,
			thickness,
			indicatorLength
		);

		g.setColour(juce::Colour(0xff48bf53));
		g.fillPath(
			indicator,
			juce::AffineTransform::rotation(angle)
			.translated(centerX, centerY)
		);
	}
};

static FlatKnobLook knobLook;

void CooeeAudioProcessorEditor::configureKnob(juce::Slider& s)
{
	s.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
	s.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 18);

	s.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
	s.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
	s.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);

	s.setLookAndFeel(&knobLook);
}

CooeeAudioProcessorEditor::CooeeAudioProcessorEditor(CooeeAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p)
{
	configureKnob(timeSlider);
	configureKnob(feedbackSlider);
	configureKnob(mixSlider);
	configureKnob(lowCutSlider);
	configureKnob(highCutSlider);

	timeSlider.setTextValueSuffix(" ms");
	feedbackSlider.setTextValueSuffix(" %");
	mixSlider.setTextValueSuffix(" %");
	lowCutSlider.setTextValueSuffix(" Hz");
	highCutSlider.setTextValueSuffix(" Hz");

	configureLabel(timeLabel, "Time");
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

	setSize(900, 320);
}

CooeeAudioProcessorEditor::~CooeeAudioProcessorEditor() {}

void CooeeAudioProcessorEditor::paint(juce::Graphics& g)
{
	g.fillAll(juce::Colour(0xff11823b));

	g.setColour(juce::Colour(0xff48bf53));
	g.fillRoundedRectangle(
		20.0f,
		100.0f,
		(float)getWidth() - 40.0f,
		180.0f,
		12.0f
	);

	g.setColour(juce::Colours::white);
	g.setFont(makeBebasFont(68.0f));

	g.drawText(
		"COOEE DELAY",
		20,
		8,
		getWidth() - 40,
		88,
		juce::Justification::centredLeft
	);

	g.setFont(makeMontserratFont(20.0f, true));
	g.setColour(juce::Colours::white);

	g.drawText(
		"michaelriley.au",
		getWidth() - 260,
		16,
		240,
		40,
		juce::Justification::centredRight
	);
}

void CooeeAudioProcessorEditor::resized()
{
	auto area = getLocalBounds().reduced(40);

	area.removeFromTop(100);

	int knobSize = 120;
	int spacing = (area.getWidth() - knobSize * 5) / 4;

	int x = area.getX();
	int y = area.getY();

	timeLabel.setBounds(x, y - 30, knobSize, 25);
	timeSlider.setBounds(x, y, knobSize, knobSize);
	x += knobSize + spacing;

	feedbackLabel.setBounds(x, y - 30, knobSize, 25);
	feedbackSlider.setBounds(x, y, knobSize, knobSize);
	x += knobSize + spacing;

	mixLabel.setBounds(x, y - 30, knobSize, 25);
	mixSlider.setBounds(x, y, knobSize, knobSize);
	x += knobSize + spacing;

	lowCutLabel.setBounds(x, y - 30, knobSize, 25);
	lowCutSlider.setBounds(x, y, knobSize, knobSize);
	x += knobSize + spacing;

	highCutLabel.setBounds(x, y - 30, knobSize, 25);
	highCutSlider.setBounds(x, y, knobSize, knobSize);
}
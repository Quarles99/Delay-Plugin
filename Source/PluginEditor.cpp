/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DelayPluginAudioProcessorEditor::DelayPluginAudioProcessorEditor (DelayPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //Delay

	delayLabel.setText("Delay", juce::dontSendNotification);
	delayLabel.setJustificationType(juce::Justification::centred);
    delayLabel.attachToComponent(&delaySlider, false);
	addAndMakeVisible(delayLabel);

	delaySlider.setSliderStyle(juce::Slider::Rotary);
	delaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    delaySlider.textFromValueFunction = [](double value) {
        return juce::String((int)value) + " ms";
        };
	addAndMakeVisible(delaySlider);

    //Feedback

    feedbackLabel.setText("Feedback", juce::dontSendNotification);
    feedbackLabel.setJustificationType(juce::Justification::centred);
    feedbackLabel.attachToComponent(&feedbackSlider, false);
    addAndMakeVisible(feedbackLabel);

    feedbackSlider.setSliderStyle(juce::Slider::Rotary);
    feedbackSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    feedbackSlider.textFromValueFunction = [](double value)
        {
            return juce::String((int)(value * 100.0)) + " %";
        };
    addAndMakeVisible(feedbackSlider);

    //Mix

    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.setJustificationType(juce::Justification::centred);
    mixLabel.attachToComponent(&mixSlider, false);
    addAndMakeVisible(mixLabel);

    mixSlider.setSliderStyle(juce::Slider::Rotary);
    mixSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    mixSlider.textFromValueFunction = [](double value)
        {
            return juce::String((int)(value * 100.0)) + " %";
        };
    addAndMakeVisible(mixSlider);

    // Attachments
    delayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, DelayPluginAudioProcessor::paramDelayTime, delaySlider);

    feedbackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, DelayPluginAudioProcessor::paramFeedback, feedbackSlider);

    mixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, DelayPluginAudioProcessor::paramMix, mixSlider);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);
}

DelayPluginAudioProcessorEditor::~DelayPluginAudioProcessorEditor()
{
}

//==============================================================================
void DelayPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void DelayPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto knobArea = getLocalBounds().reduced(20);

    delaySlider.setBounds(knobArea.removeFromLeft(120));
	feedbackSlider.setBounds(knobArea.removeFromLeft(120));
	mixSlider.setBounds(knobArea.removeFromLeft(120));

}

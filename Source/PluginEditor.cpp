/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HiJUCEAudioProcessorEditor::HiJUCEAudioProcessorEditor (HiJUCEAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(dial);
    dial.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    dial.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 32);
    dial.setRange(-24.0, 24.0, 0.1);
    dial.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::whitesmoke.withAlpha(0.5f));
    dial.setDoubleClickReturnValue(true, 0.0);
    
    setResizable(true, true);
    setResizeLimits(500, 250, 1500, 750);
    getConstrainer()->setFixedAspectRatio(2.0);
    
    setSize (1000, 500);
}

HiJUCEAudioProcessorEditor::~HiJUCEAudioProcessorEditor()
{
}

//==============================================================================
void HiJUCEAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (juce::Colours::black);

    g.setColour (juce::Colours::green);
    g.setFont (juce::FontOptions (36.0f));
    g.drawFittedText ("Hello Worldz!", getLocalBounds(), juce::Justification::centredTop, 1);
}

void HiJUCEAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto leftMargin = getWidth() * 0.02;
    auto topMargin = getHeight() * 0.02;
    dial.setBounds(leftMargin, topMargin, 72, 256);
}

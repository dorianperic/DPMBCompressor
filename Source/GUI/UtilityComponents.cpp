/*
  ==============================================================================

    UtilityComponents.cpp
    Created: 11 May 2022 5:39:07pm
    Author:  Dorian

  ==============================================================================
*/

#include "UtilityComponents.h"

Placeholder::Placeholder() {

    juce::Random r;
    customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
}

void Placeholder::paint(juce::Graphics& g){
    g.fillAll(customColor);
}

RotarySlider::RotarySlider() :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
        juce::Slider::TextEntryBoxPosition::NoTextBox)
{}

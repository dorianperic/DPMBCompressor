/*
  ==============================================================================

    UtilityComponents.h
    Created: 11 May 2022 5:39:07pm
    Author:  Dorian

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>


struct Placeholder : juce::Component {
    Placeholder();

    void paint(juce::Graphics& g) override;

    juce::Colour customColor;
};

struct RotarySlider : juce::Slider {
    RotarySlider();
};

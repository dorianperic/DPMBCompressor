/*
  ==============================================================================

    LookAndFeel.h
    Created: 11 May 2022 5:24:45pm
    Author:  Dorian

  ==============================================================================
*/
#include <JuceHeader.h>

#define USE_LIVE_CONSTANT true

#if USE_LIVE_CONSTANT
#define colorHelper(c) JUCE_LIVE_CONSTANT(c);
#else 
#define colorHelper(c) c;
#endif

namespace ColorScheme {
    inline juce::Colour getSliderBorderColor() {
        return colorHelper(juce::Colour(0xff0bcb50));
    }
    inline juce::Colour getModuleBorderColor() {
        return colorHelper(juce::Colour(0xfff2f214));
    }
    inline juce::Colour getRotaryLabelsColor() {
        return colorHelper(juce::Colour(0xfff2f214));
    }

    
}

struct LookAndFeel : juce::LookAndFeel_V4
{
    void drawRotarySlider(juce::Graphics&,
        int x, int y, int width, int height,
        float sliderPosProportional,
        float rotaryStartAngle,
        float rotaryEndAngle,
        juce::Slider&) override;

    void drawToggleButton(juce::Graphics& g,
        juce::ToggleButton& toggleButton,
        bool shouldDrawButtonAsHighlighted,
        bool shouldDrawButtonAsDown) override;
};

#pragma once

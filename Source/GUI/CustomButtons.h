/*
  ==============================================================================

    CustomButtons.h
    Created: 11 May 2022 5:35:51pm
    Author:  Dorian

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>


struct PowerButton : juce::ToggleButton { };

struct AnalyzerButton : juce::ToggleButton
{
    void resized() override;

    juce::Path randomPath;
};


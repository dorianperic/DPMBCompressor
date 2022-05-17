/*
  ==============================================================================

    Utilities.cpp
    Created: 13 May 2022 2:50:14pm
    Author:  Dorian

  ==============================================================================
*/

#include "Utilities.h"


juce::String getValString(const juce::RangedAudioParameter& param,
    bool getLow,
    juce::String suffix)
{

    juce::String str;

    auto val = getLow ? param.getNormalisableRange().start :
        param.getNormalisableRange().end;

    bool useK = addKiloValue(val);
    str << val;

    if (useK)
        str << "k";

    str << suffix;

    return str;
}

juce::Rectangle<int> drawModuleBackground(juce::Graphics& g,
    juce::Rectangle<int> bounds) {
    
    using namespace juce;
    g.setColour(Colours::blueviolet);
    g.fillAll();

    auto localBounds = bounds;

    bounds.reduce(2, 2);
    g.setColour(Colours::black);
    g.fillRoundedRectangle(bounds.toFloat(), 2);

    g.drawRect(localBounds);

    return bounds;
}

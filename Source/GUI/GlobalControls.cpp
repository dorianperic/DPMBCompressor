/*
  ==============================================================================

    GlobalControls.cpp
    Created: 13 May 2022 3:00:54pm
    Author:  Dorian

  ==============================================================================
*/

#include "GlobalControls.h"
#include "Utilities.h"
#include "../DSP/Params.h"

GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts) {
    using namespace Params;
    const auto& params = GetParams();

    auto getParamHelper = [&params, &apvts](const auto& name) -> auto& {
        return getParam(apvts, params, name);
    };

    // Cashiranje postavki za ustedu na vremenu
    auto& gainInParam = getParamHelper(Names::Gain_In);
    auto& lowMidParam = getParamHelper(Names::Low_Mid_Crossover_Freq);
    auto& midHighParam = getParamHelper(Names::Mid_High_Crossover_Freq);
    auto& gainOutParam = getParamHelper(Names::Gain_Out);

    inGainSlider = std::make_unique<RSWL>(&gainInParam, "dB", "INPUT TRIM");
    lowMidXoverSlider = std::make_unique<RSWL>(&lowMidParam, "Hz", "LOW-MID X-OVER");
    midHighXoverSlider = std::make_unique<RSWL>(&midHighParam, "Hz", "MID-HIGH X-OVER");
    outGainSlider = std::make_unique<RSWL>(&gainOutParam, "dB", "OUTPUT TRIM");

    auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto& name, auto& slider) {
        makeAttachment(attachment, apvts, params, name, slider);
    };

    // Povezivanje sa DSP-om
    makeAttachmentHelper(inGainSliderAttachment,
        Names::Gain_In,
        *inGainSlider);
    makeAttachmentHelper(lowMidXoverSliderAttachment,
        Names::Low_Mid_Crossover_Freq,
        *lowMidXoverSlider);
    makeAttachmentHelper(midHighXoverSliderAttachment,
        Names::Mid_High_Crossover_Freq,
        *midHighXoverSlider);
    makeAttachmentHelper(outGainSliderAttachment,
        Names::Gain_Out,
        *outGainSlider);

    // Dodavanje valuea na slidere
    addLabelPairs(inGainSlider->labels,
        gainInParam,
        "dB");
    addLabelPairs(lowMidXoverSlider->labels,
        lowMidParam,
        "Hz");
    addLabelPairs(midHighXoverSlider->labels,
        midHighParam,
        "Hz");
    addLabelPairs(outGainSlider->labels,
        gainOutParam,
        "dB");

    addAndMakeVisible(*inGainSlider);
    addAndMakeVisible(*lowMidXoverSlider);
    addAndMakeVisible(*midHighXoverSlider);
    addAndMakeVisible(*outGainSlider);
}

void GlobalControls::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}

void GlobalControls::resized() {
    auto bounds = getLocalBounds().reduced(5);
    using namespace juce;

    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);

    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(*inGainSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*lowMidXoverSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*midHighXoverSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*outGainSlider).withFlex(1.f));
    flexBox.items.add(endCap);

    flexBox.performLayout(bounds);
}
/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "../Source/DSP/Params.h"

ControlBar::ControlBar() {
    analyzerButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(analyzerButton);

    addAndMakeVisible(globalBypassButton);
}

void ControlBar::resized() {
    auto bounds = getLocalBounds();

    analyzerButton.setBounds(bounds.removeFromLeft(50)
                             .withTrimmedTop(4)
                             .withTrimmedBottom(4));

    globalBypassButton.setBounds(bounds.removeFromRight(60)
                                 .withTrimmedTop(2)
                                 .withTrimmedBottom(2));
}

DPMBCompressorAudioProcessorEditor::DPMBCompressorAudioProcessorEditor (DPMBCompressorAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&lnf);

    controlBar.analyzerButton.onClick = [this]() {
        auto shouldBeOn = controlBar.analyzerButton.getToggleState();
        analyzer.toggleAnalysisEnablement(shouldBeOn);
    };

    controlBar.globalBypassButton.onClick = [this]() {
        toggleGlobalBypassState();
    };

    addAndMakeVisible(controlBar);
    addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);

    setSize (612, 510);

    startTimerHz(60);
}

DPMBCompressorAudioProcessorEditor::~DPMBCompressorAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void DPMBCompressorAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setColour (juce::Colours::white);
    //g.setFont (15.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);

    g.fillAll(juce::Colours::black);
}

void DPMBCompressorAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

    auto bounds = getLocalBounds();

    controlBar.setBounds(bounds.removeFromTop(32));
    bandControls.setBounds(bounds.removeFromBottom(135));
    analyzer.setBounds(bounds.removeFromTop(225));
    globalControls.setBounds(bounds);
}

void DPMBCompressorAudioProcessorEditor::timerCallback() {
    std::vector<float> values{
        audioProcessor.lowBandComp.getRmsInputLevelDb(),
        audioProcessor.lowBandComp.getRmsOutputLevelDb(),
        audioProcessor.midBandComp.getRmsInputLevelDb(),
        audioProcessor.midBandComp.getRmsOutputLevelDb(),
        audioProcessor.highBandComp.getRmsInputLevelDb(),
        audioProcessor.highBandComp.getRmsOutputLevelDb(),
    };

    analyzer.update(values);

    // Global button bypass enable ako su svi bandovi disableani
    updateGlobalBypassButton();
}

void DPMBCompressorAudioProcessorEditor::updateGlobalBypassButton() {
    auto params = getBypassParams();

    bool allBandsAreBypassed = std::all_of(params.begin(),
                                           params.end(),
                                           [](const auto& param) {return param->get(); });

    controlBar.globalBypassButton.setToggleState(allBandsAreBypassed,
                                                 juce::NotificationType::dontSendNotification);
}

void DPMBCompressorAudioProcessorEditor::toggleGlobalBypassState() {
    auto shouldEnableEverything = !controlBar.globalBypassButton.getToggleState();

    auto params = getBypassParams();

    // Bypass helper
    auto bypassParamHelper = [](auto* param, bool shouldBeBypassed) {
        param->beginChangeGesture();
        param->setValueNotifyingHost(shouldBeBypassed ? 1.f : 0.f);
        param->endChangeGesture();
    };

    // Global Bypass funkcionalnost
    for (auto* param : params) {
        bypassParamHelper(param, !shouldEnableEverything);
    }

    // Bypass za sve bandove reflect
    bandControls.toggleAllBands(!shouldEnableEverything);
}

std::array<juce::AudioParameterBool*, 3> DPMBCompressorAudioProcessorEditor::getBypassParams() {
    using namespace Params;
    using namespace juce;
    const auto& params = Params::GetParams();
    auto& apvts = audioProcessor.apvts;

    auto boolHelper = [&apvts, &params](const auto& paramName) {
        auto param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);

        return param;
    };

    auto* lowBypassParam = boolHelper(Names::Bypassed_Low_Band);
    auto* midBypassParam = boolHelper(Names::Bypassed_Mid_Band);
    auto* highBypassParam = boolHelper(Names::Bypassed_High_Band);

    return{
        lowBypassParam,
        midBypassParam,
        highBypassParam
    };
}

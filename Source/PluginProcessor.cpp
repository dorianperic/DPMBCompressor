/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DPMBCompressorAudioProcessor::DPMBCompressorAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    using namespace Params;
    const auto& params = GetParams();
    

    // U lamda f-ma compiler zbog auto flaga sam vadi tip varijable
    auto floatHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName) {
        param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };
    auto choiceHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName) {
        param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };
    auto boolHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName) {
        param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    // Cashiranje postavki kompresora sa osiguranjem
    floatHelper(lowBandComp.attack, Names::Attack_Low_Band);
    floatHelper(lowBandComp.release, Names::Release_Low_Band);
    floatHelper(lowBandComp.threshold, Names::Threshold_Low_Band);

    floatHelper(midBandComp.attack, Names::Attack_Mid_Band);
    floatHelper(midBandComp.release, Names::Release_Mid_Band);
    floatHelper(midBandComp.threshold, Names::Threshold_Mid_Band);

    floatHelper(highBandComp.attack, Names::Attack_High_Band);
    floatHelper(highBandComp.release, Names::Release_High_Band);
    floatHelper(highBandComp.threshold, Names::Threshold_High_Band);
    
    choiceHelper(lowBandComp.ratio, Names::Ratio_Low_Band);
    choiceHelper(midBandComp.ratio, Names::Ratio_Mid_Band);
    choiceHelper(highBandComp.ratio, Names::Ratio_High_Band);

    boolHelper(lowBandComp.bypassed, Names::Bypassed_Low_Band);
    boolHelper(midBandComp.bypassed, Names::Bypassed_Mid_Band);
    boolHelper(highBandComp.bypassed, Names::Bypassed_High_Band);

    floatHelper(lowMidCrossover, Names::Low_Mid_Crossover_Freq);
    floatHelper(midHighCrossover, Names::Mid_High_Crossover_Freq);


    LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

    AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);

    LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

    //invAP1.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    //invAP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);


}

DPMBCompressorAudioProcessor::~DPMBCompressorAudioProcessor()
{
}

//==============================================================================
const juce::String DPMBCompressorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DPMBCompressorAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DPMBCompressorAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DPMBCompressorAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DPMBCompressorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DPMBCompressorAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DPMBCompressorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DPMBCompressorAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String DPMBCompressorAudioProcessor::getProgramName (int index)
{
    return {};
}

void DPMBCompressorAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void DPMBCompressorAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    for( auto& comp : compressors)
        comp.prepare(spec);

    LP1.prepare(spec);
    HP1.prepare(spec);

    AP2.prepare(spec);

    LP2.prepare(spec);
    HP2.prepare(spec);

    //invAP1.prepare(spec);
    //invAP2.prepare(spec);

    //invAPBuffer.setSize(spec.numChannels, samplesPerBlock);

    for (auto& buffer : filterBuffers)
    {
        buffer.setSize(spec.numChannels, samplesPerBlock);
    }
}

void DPMBCompressorAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DPMBCompressorAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DPMBCompressorAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for( auto& compressor : compressors)
        compressor.updateCompressorSettings();

    //compressor.process(buffer);

    for (auto& fb : filterBuffers)
    {
        fb = buffer;
    }

    //invAPBuffer = buffer;

    auto lowMidCutoffFreq = lowMidCrossover->get();
    LP1.setCutoffFrequency(lowMidCutoffFreq);
    HP1.setCutoffFrequency(lowMidCutoffFreq);
    //invAP1.setCutoffFrequency(lowMidCutoffFreq);

    auto midHighCutoffFreq = midHighCrossover->get();
    AP2.setCutoffFrequency(midHighCutoffFreq);
    LP2.setCutoffFrequency(midHighCutoffFreq);
    HP2.setCutoffFrequency(midHighCutoffFreq);
    //invAP2.setCutoffFrequency(midHighCutoffFreq);


    // Tri blocka za low,mid i high
    auto filterBuffer0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto filterBuffer1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto filterBuffer2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);


    // Dva contexta za low i mid
    auto filterBuffer0Context = juce::dsp::ProcessContextReplacing<float>(filterBuffer0Block);
    auto filterBuffer1Context = juce::dsp::ProcessContextReplacing<float>(filterBuffer1Block);
    auto filterBuffer2Context = juce::dsp::ProcessContextReplacing<float>(filterBuffer2Block);

    // Procesiranje contexta
    LP1.process(filterBuffer0Context);
    AP2.process(filterBuffer0Context);

    HP1.process(filterBuffer1Context);
    filterBuffers[2] = filterBuffers[1];
    LP2.process(filterBuffer1Context);
    
    HP2.process(filterBuffer2Context);

    for (size_t i = 0; i < filterBuffers.size(); ++i) {
        compressors[i].process(filterBuffers[i]);
    }

    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    buffer.clear();

    // Lamda f-ja 
    auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source) {
        for (auto i = 0; i < nc; ++i) {
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
        }
    };

    addFilterBand(buffer, filterBuffers[0]);
    addFilterBand(buffer, filterBuffers[1]);
    addFilterBand(buffer, filterBuffers[2]);


}

//==============================================================================
bool DPMBCompressorAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* DPMBCompressorAudioProcessor::createEditor()
{
    //return new DPMBCompressorAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void DPMBCompressorAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.

    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void DPMBCompressorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.

    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout DPMBCompressorAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;

    using namespace juce;
    using namespace Params;
    const auto& params = GetParams();

    auto attackReleaseRange = NormalisableRange<float>(5, 500, 1, 1);

    #pragma region Threshold sliders

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Threshold_Low_Band),
                                                     params.at(Names::Threshold_Low_Band),
                                                     NormalisableRange<float>(-60, 12, 1, 1),
                                                     0));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Threshold_Mid_Band),
                                                     params.at(Names::Threshold_Mid_Band),
                                                     NormalisableRange<float>(-60, 12, 1, 1),
                                                     0));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Threshold_High_Band),
                                                     params.at(Names::Threshold_High_Band),
                                                     NormalisableRange<float>(-60, 12, 1, 1),
                                                     0));

    #pragma endregion

    #pragma region Attack sliders

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Attack_Low_Band),
                                                     params.at(Names::Attack_Low_Band),
                                                     attackReleaseRange,
                                                     50));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Attack_Mid_Band),
                                                     params.at(Names::Attack_Mid_Band),
                                                     attackReleaseRange,
                                                     50));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Attack_High_Band),
                                                     params.at(Names::Attack_High_Band),
                                                     attackReleaseRange,
                                                     50));

    #pragma endregion

    #pragma region Release sliders

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Release_Low_Band),
                                                     params.at(Names::Release_Low_Band),
                                                     attackReleaseRange,
                                                     250));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Release_Mid_Band),
                                                     params.at(Names::Release_Mid_Band),
                                                     attackReleaseRange,
                                                     250));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Release_High_Band),
                                                     params.at(Names::Release_High_Band),
                                                     attackReleaseRange,
                                                     250));

    #pragma endregion

    auto choices = std::vector<double>{ 1, 1.5 , 2 ,3 ,4 ,5 ,6 ,7 ,8, 9, 10, 15, 20, 50 ,100 };

    juce::StringArray sa;

    for (auto choice : choices)
    {
        sa.add(juce::String(choice, 1));
    }

    #pragma region Ratio sliders

    layout.add(std::make_unique<AudioParameterChoice>(params.at(Names::Ratio_Low_Band),
                                                      params.at(Names::Ratio_Low_Band),
                                                      sa,
                                                      3));

    layout.add(std::make_unique<AudioParameterChoice>(params.at(Names::Ratio_Mid_Band),
                                                      params.at(Names::Ratio_Mid_Band),
                                                      sa,
                                                      3));

    layout.add(std::make_unique<AudioParameterChoice>(params.at(Names::Ratio_High_Band),
                                                      params.at(Names::Ratio_High_Band),
                                                      sa,
                                                      3));

    #pragma endregion

    #pragma region Bypass buttons

    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::Bypassed_Low_Band),
                                                    params.at(Names::Bypassed_Low_Band),
                                                    false));
    
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::Bypassed_Mid_Band),
                                                    params.at(Names::Bypassed_Mid_Band),
                                                    false));

    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::Bypassed_High_Band),
                                                    params.at(Names::Bypassed_High_Band),
                                                    false));

    #pragma endregion

    #pragma region Crossover freqs

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Low_Mid_Crossover_Freq),
                                                     params.at(Names::Low_Mid_Crossover_Freq),
                                                     NormalisableRange<float>(20, 999, 1, 1),
                                                     400));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::Mid_High_Crossover_Freq),
                                                     params.at(Names::Mid_High_Crossover_Freq),
                                                     NormalisableRange<float>(1000, 20000, 1, 1),
                                                     2000));

    #pragma endregion


    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DPMBCompressorAudioProcessor();
}

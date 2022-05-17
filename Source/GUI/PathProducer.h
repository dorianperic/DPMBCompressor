/*
  ==============================================================================

    PathProducer.h
    Created: 15 May 2022 11:34:43pm
    Author:  Dorian

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include <../Source/GUI/FFTDataGenerator.h>
#include <../Source/GUI/AnalyzerPathGenerator.h>
#include <../Source/DSP/SingleChannelSampleFifo.h>
#include "../PluginProcessor.h"



struct PathProducer
{
    PathProducer(SingleChannelSampleFifo<DPMBCompressorAudioProcessor::BlockType>& scsf) :
        leftChannelFifo(&scsf)
    {
        leftChannelFFTDataGenerator.changeOrder(FFTOrder::order2048);
        monoBuffer.setSize(1, leftChannelFFTDataGenerator.getFFTSize());
    }
    void process(juce::Rectangle<float> fftBounds, double sampleRate);
    juce::Path getPath() { return leftChannelFFTPath; }

    void updateNegativeInfinity(float nf) { negativeInfinity = nf; }
private:
    SingleChannelSampleFifo<DPMBCompressorAudioProcessor::BlockType>* leftChannelFifo;

    juce::AudioBuffer<float> monoBuffer;

    FFTDataGenerator<std::vector<float>> leftChannelFFTDataGenerator;

    AnalyzerPathGenerator<juce::Path> pathProducer;

    juce::Path leftChannelFFTPath;

    float negativeInfinity{ -48.f };
};


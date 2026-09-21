#pragma once
#include <JuceHeader.h>

class CardizOneAudioProcessor final : public juce::AudioProcessor {
public:
  CardizOneAudioProcessor();
  void prepareToPlay(double, int) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout&) const override;
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }
  const juce::String getName() const override { return "CARDIZ ONE"; }
  bool acceptsMidi() const override { return false; }
  bool producesMidi() const override { return false; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 0.0; }
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return {}; }
  void changeProgramName(int,const juce::String&) override {}
  void getStateInformation(juce::MemoryBlock&) override;
  void setStateInformation(const void*, int) override;

  juce::AudioProcessorValueTreeState apvts;
  std::atomic<float> inputPeak {0}, outputPeak {0}, loudness {-60};
  static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

private:
  juce::dsp::StateVariableTPTFilter<float> lowCut;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> presenceFilter;
  juce::dsp::Compressor<float> compressor;
  juce::dsp::Limiter<float> limiter;
  juce::SmoothedValue<float> drive, width;
  double currentSampleRate = 44100.0;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CardizOneAudioProcessor)
};

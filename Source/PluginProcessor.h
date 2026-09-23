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
  std::atomic<float> analysisProgress {0}, analysedRms {-60}, analysedPeak {-60};
  std::atomic<float> analysedLow {0}, analysedMid {0}, analysedHigh {0};
  std::atomic<float> analysedCrest {0}, analysedSibilance {0};
  std::atomic<float> analysedResonanceHz {0}, analysedResonanceSeverity {0};
  std::atomic<bool> analysisRunning {false}, analysisReady {false};
  static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
  void beginAnalysis();
  bool applyAnalysisResult();

private:
  juce::dsp::StateVariableTPTFilter<float> lowCut;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> presenceFilter;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> bodyFilter;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> airFilter;
  juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> resonanceFilter;
  juce::dsp::Compressor<float> compressor;
  juce::dsp::Limiter<float> limiter;
  juce::SmoothedValue<float> drive, width, makeup, abGain, comparisonMix;
  juce::AudioBuffer<float> dryBuffer;
  double currentSampleRate = 44100.0;
  double analysisSamples = 0, sumSquares = 0, lowSquares = 0, midSquares = 0, highSquares = 0;
  double analysisTargetSamples = 220500.0;
  double resonanceSquares[4] {0, 0, 0, 0};
  double sibilanceSquares = 0;
  float analysisPeakValue = 0;
  double momentaryEnergy = 0, wetMatchEnergy = 0, dryMatchEnergy = 0;
  float analyzer150 = 0, analyzer400 = 0, analyzer900 = 0;
  float analyzer2500 = 0, analyzer5000 = 0, analyzer10000 = 0;
  float deEssLowpass[2] {0, 0}, deEssEnvelope[2] {0, 0};
  int cachedMode = -1, cachedStyle = -1;
  float cachedTonal = -1, cachedResonanceHz = -1, cachedResonanceCut = -1;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CardizOneAudioProcessor)
};

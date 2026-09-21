#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class GoldLookAndFeel final : public juce::LookAndFeel_V4 {
public:
  GoldLookAndFeel();
  void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
  void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
};

class CardizOneAudioProcessorEditor final : public juce::AudioProcessorEditor,private juce::Timer {
public:
  explicit CardizOneAudioProcessorEditor(CardizOneAudioProcessor&);
  ~CardizOneAudioProcessorEditor() override;
  void paint(juce::Graphics&) override;
  void resized() override;
private:
  void timerCallback() override;
  CardizOneAudioProcessor& processor;
  GoldLookAndFeel look;
  juce::Slider tonal,glue,punch,width;
  juce::TextButton vox{"ONE VOX"},master{"ONE MASTER"},pro{"PRO"},finish{"FINISH"};
  using SA=juce::AudioProcessorValueTreeState::SliderAttachment;
  std::unique_ptr<SA> aTonal,aGlue,aPunch,aWidth;
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CardizOneAudioProcessorEditor)
};

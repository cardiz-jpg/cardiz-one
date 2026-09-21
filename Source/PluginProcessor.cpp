#include "PluginProcessor.h"
#include "PluginEditor.h"

CardizOneAudioProcessor::CardizOneAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
                                  .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
  apvts(*this,nullptr,"STATE",createLayout()) {}

auto CardizOneAudioProcessor::createLayout() -> juce::AudioProcessorValueTreeState::ParameterLayout {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
  p.push_back(std::make_unique<juce::AudioParameterChoice>("mode","Mode",juce::StringArray{"ONE VOX","ONE MASTER","PRO"},0));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("tonal","Tonal Balance",0.f,100.f,60.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("glue","Glue",0.f,100.f,68.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("punch","Punch",0.f,100.f,72.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("width","Width",0.f,100.f,56.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("ceiling","True Peak Ceiling",-2.f,-0.1f,-1.f));
  p.push_back(std::make_unique<juce::AudioParameterBool>("analyze","Analyze",false));
  p.push_back(std::make_unique<juce::AudioParameterBool>("bypass","Bypass",false));
  return {p.begin(),p.end()};
}

void CardizOneAudioProcessor::prepareToPlay(double sr,int block) {
  currentSampleRate=sr;
  juce::dsp::ProcessSpec spec{sr,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};
  lowCut.prepare(spec); presenceFilter.prepare(spec); compressor.prepare(spec); limiter.prepare(spec);
  lowCut.setType(juce::dsp::StateVariableTPTFilterType::highpass); lowCut.setCutoffFrequency(65.f);
  *presenceFilter.state=*juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr,3800.f,.7071f,1.f);
  drive.reset(sr,.04); width.reset(sr,.04);
}

bool CardizOneAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
  return l.getMainInputChannelSet()==l.getMainOutputChannelSet() &&
    (l.getMainOutputChannelSet()==juce::AudioChannelSet::mono() || l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo());
}

void CardizOneAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {
  juce::ScopedNoDenormals noDenormals;
  auto in=b.getMagnitude(0,b.getNumSamples()); inputPeak.store(in);
  if (apvts.getRawParameterValue("bypass")->load()>.5f) { outputPeak.store(in); return; }
  const float tonal=apvts.getRawParameterValue("tonal")->load()/100.f;
  const float glue=apvts.getRawParameterValue("glue")->load()/100.f;
  const float punch=apvts.getRawParameterValue("punch")->load()/100.f;
  const float wide=apvts.getRawParameterValue("width")->load()/100.f;
  const int mode=(int)apvts.getRawParameterValue("mode")->load();
  *presenceFilter.state=*juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate,3800.f,.7071f,juce::Decibels::decibelsToGain((tonal-.5f)*5.f));
  compressor.setThreshold(-8.f-glue*16.f); compressor.setRatio(1.5f+glue*3.5f);
  compressor.setAttack(24.f-punch*20.f); compressor.setRelease(70.f+glue*160.f);
  limiter.setThreshold(apvts.getRawParameterValue("ceiling")->load()); limiter.setRelease(90.f);
  drive.setTargetValue(1.f+(mode==0?.8f:1.4f)*glue); width.setTargetValue(.7f+wide*.8f);
  juce::dsp::AudioBlock<float> block(b); juce::dsp::ProcessContextReplacing<float> ctx(block);
  lowCut.process(ctx); presenceFilter.process(ctx); compressor.process(ctx);
  for(int c=0;c<b.getNumChannels();++c) for(int n=0;n<b.getNumSamples();++n)
    b.setSample(c,n,std::tanh(b.getSample(c,n)*drive.getNextValue()));
  if(b.getNumChannels()==2) for(int n=0;n<b.getNumSamples();++n) {
    float l=b.getSample(0,n),r=b.getSample(1,n),m=.5f*(l+r),s=.5f*(l-r)*width.getNextValue();
    b.setSample(0,n,m+s); b.setSample(1,n,m-s);
  }
  limiter.process(ctx);
  float out=b.getMagnitude(0,b.getNumSamples()); outputPeak.store(out);
  loudness.store(juce::jlimit(-60.f,0.f,juce::Decibels::gainToDecibels(out)-3.f));
}

void CardizOneAudioProcessor::getStateInformation(juce::MemoryBlock& d) { if(auto x=apvts.copyState().createXml()) copyXmlToBinary(*x,d); }
void CardizOneAudioProcessor::setStateInformation(const void* d,int s) { if(auto x=getXmlFromBinary(d,s)) apvts.replaceState(juce::ValueTree::fromXml(*x)); }
juce::AudioProcessorEditor* CardizOneAudioProcessor::createEditor(){ return new CardizOneAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){ return new CardizOneAudioProcessor(); }

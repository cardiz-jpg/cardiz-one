#include "PluginProcessor.h"
#include "PluginEditor.h"

CardizOneAudioProcessor::CardizOneAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
                                  .withOutput("Output",juce::AudioChannelSet::stereo(),true)),
  apvts(*this,nullptr,"STATE",createLayout()) {}

auto CardizOneAudioProcessor::createLayout() -> juce::AudioProcessorValueTreeState::ParameterLayout {
  std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
  p.push_back(std::make_unique<juce::AudioParameterChoice>("mode","Mode",juce::StringArray{"ONE VOX","ONE MASTER","PRO"},0));
  p.push_back(std::make_unique<juce::AudioParameterChoice>("voiceStyle","Voice Style",juce::StringArray{"LATINO URBANO","ELECTRONICA","BALADA","NATURAL"},0));
  p.push_back(std::make_unique<juce::AudioParameterChoice>("masterStyle","Master Style",juce::StringArray{"STREAMING MODERNO","CLUB / ELECTRONICA","BALADA / ORGANICO","TRANSPARENTE"},0));
  p.push_back(std::make_unique<juce::AudioParameterChoice>("proStyle","Pro Strategy",juce::StringArray{"IMPACTO CONTROLADO","DINAMICA ABIERTA","BALANCE CALIDO","REFERENCIA NEUTRA"},0));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("tonal","Tonal Balance",0.f,100.f,60.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("glue","Glue",0.f,100.f,68.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("punch","Punch",0.f,100.f,72.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("width","Width",0.f,100.f,56.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("ceiling","Limiter Ceiling",-2.f,-0.1f,-1.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("makeup","Output Gain",-6.f,12.f,0.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("deEss","De-ess",0.f,100.f,0.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("resonanceHz","Resonance Frequency",150.f,5000.f,1000.f));
  p.push_back(std::make_unique<juce::AudioParameterFloat>("resonanceCut","Resonance Reduction",0.f,6.f,0.f));
  p.push_back(std::make_unique<juce::AudioParameterBool>("compareDry","Matched Original",false));
  p.push_back(std::make_unique<juce::AudioParameterBool>("bypass","Bypass",false));
  return {p.begin(),p.end()};
}

void CardizOneAudioProcessor::prepareToPlay(double sr,int block) {
  currentSampleRate=sr;
  juce::dsp::ProcessSpec spec{sr,(juce::uint32)block,(juce::uint32)getTotalNumOutputChannels()};
  lowCut.prepare(spec); presenceFilter.prepare(spec); bodyFilter.prepare(spec); airFilter.prepare(spec);
  resonanceFilter.prepare(spec); compressor.prepare(spec); limiter.prepare(spec);
  lowCut.setType(juce::dsp::StateVariableTPTFilterType::highpass); lowCut.setCutoffFrequency(65.f);
  *presenceFilter.state=*juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr,3800.f,.7071f,1.f);
  *bodyFilter.state=*juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr,220.f,.8f,1.f);
  *airFilter.state=*juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr,9000.f,.7071f,1.f);
  *resonanceFilter.state=*juce::dsp::IIR::Coefficients<float>::makePeakFilter(sr,1000.f,3.f,1.f);
  drive.reset(sr,.04); width.reset(sr,.04); makeup.reset(sr,.08); abGain.reset(sr,.08);
  comparisonMix.reset(sr,.025);
  abGain.setCurrentAndTargetValue(1.f);
  comparisonMix.setCurrentAndTargetValue(apvts.getRawParameterValue("compareDry")->load()>.5f ? 1.f : 0.f);
  momentaryEnergy=wetMatchEnergy=dryMatchEnergy=0;
  deEssLowpass[0]=deEssLowpass[1]=deEssEnvelope[0]=deEssEnvelope[1]=0;
  cachedMode=cachedStyle=-1;
  cachedTonal=cachedResonanceHz=cachedResonanceCut=-1;
  dryBuffer.setSize(getTotalNumOutputChannels(),block,false,false,true);
}

void CardizOneAudioProcessor::beginAnalysis() {
  analysisRunning.store(false);
  analysisSamples=0; sumSquares=0; lowSquares=0; midSquares=0; highSquares=0;
  for(auto& e:resonanceSquares) e=0; sibilanceSquares=0;
  analysisPeakValue=0;
  analyzer150=analyzer400=analyzer900=analyzer2500=analyzer5000=analyzer10000=0;
  const int mode=(int)apvts.getRawParameterValue("mode")->load();
  analysisTargetSamples=currentSampleRate*(mode==0 ? 5.0 : 10.0);
  analysisProgress.store(0); analysisReady.store(false); analysisRunning.store(true);
}

bool CardizOneAudioProcessor::applyAnalysisResult() {
  if (!analysisReady.exchange(false)) return false;
  const auto mode=(int)apvts.getRawParameterValue("mode")->load();
  const auto style=(int)apvts.getRawParameterValue("voiceStyle")->load();
  const auto masterStyle=(int)apvts.getRawParameterValue("masterStyle")->load();
  const auto proStyle=(int)apvts.getRawParameterValue("proStyle")->load();
  const float rms=analysedRms.load(), peak=analysedPeak.load();
  const float low=analysedLow.load(), mid=analysedMid.load(), high=analysedHigh.load();
  const float sibilance=analysedSibilance.load(), resonanceSeverity=analysedResonanceSeverity.load();
  float tonal=50, glue=50, punch=50, wide=50, outputGain=0;
  if(mode==0) {
    const float baseTonal[] {70,76,58,52}, baseGlue[] {72,62,52,42};
    const float basePunch[] {78,72,48,55}, baseWidth[] {22,42,18,12};
    tonal=baseTonal[style] + (mid-low)*18.f + (0.28f-high)*12.f;
    glue=baseGlue[style] + juce::jlimit(-12.f,18.f,(-22.f-rms)*1.2f);
    punch=basePunch[style] + juce::jlimit(-12.f,12.f,((peak-rms)-10.f)*1.5f);
    wide=baseWidth[style]; outputGain=juce::jlimit(0.f,10.f,-15.f-rms);
  } else if(mode==1) {
    const float baseTonal[] {58.f,64.f,50.f,48.f};
    const float baseGlue[] {58.f,66.f,46.f,38.f};
    const float basePunch[] {62.f,74.f,50.f,46.f};
    const float baseWidth[] {56.f,68.f,46.f,42.f};
    tonal=baseTonal[masterStyle]+(mid-low)*22.f+(0.24f-high)*14.f;
    glue=juce::jlimit(30.f,78.f,baseGlue[masterStyle]+(-19.f-rms)*1.2f);
    punch=juce::jlimit(36.f,82.f,basePunch[masterStyle]+((peak-rms)-11.f)*1.1f);
    wide=juce::jlimit(28.f,76.f,baseWidth[masterStyle]+(mid-low)*8.f);
    const float targetRms[] {-14.f,-12.f,-16.f,-17.f};
    outputGain=juce::jlimit(-2.f,8.f,targetRms[masterStyle]-rms);
  } else {
    const float baseTonal[] {54.f,48.f,44.f,50.f};
    const float baseGlue[] {48.f,32.f,42.f,28.f};
    const float basePunch[] {64.f,48.f,44.f,46.f};
    const float baseWidth[] {54.f,48.f,42.f,46.f};
    tonal=baseTonal[proStyle]+(mid-low)*14.f+(0.24f-high)*9.f;
    glue=juce::jlimit(20.f,68.f,baseGlue[proStyle]+(-20.f-rms));
    punch=juce::jlimit(30.f,76.f,basePunch[proStyle]+((peak-rms)-11.f));
    wide=juce::jlimit(25.f,68.f,baseWidth[proStyle]+(mid-low)*6.f);
    const float targetRms[] {-14.f,-17.f,-16.f,-18.f};
    outputGain=juce::jlimit(-2.f,6.f,targetRms[proStyle]-rms);
  }
  auto set=[this](const char* id,float v,float lo,float hi){
    if(auto* p=apvts.getParameter(id)) p->setValueNotifyingHost(p->convertTo0to1(juce::jlimit(lo,hi,v)));
  };
  set("tonal",tonal,0,100); set("glue",glue,0,100); set("punch",punch,0,100); set("width",wide,0,100);
  set("makeup",outputGain,-6,12);
  set("deEss",juce::jlimit(0.f,75.f,(sibilance-.025f)*900.f),0,100);
  set("resonanceHz",analysedResonanceHz.load(),150,5000);
  set("resonanceCut",juce::jlimit(0.f,6.f,(resonanceSeverity-1.25f)*2.5f),0,6);
  if(auto* p=apvts.getParameter("compareDry")) p->setValueNotifyingHost(0.f);
  return true;
}

bool CardizOneAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
  return l.getMainInputChannelSet()==l.getMainOutputChannelSet() &&
    (l.getMainOutputChannelSet()==juce::AudioChannelSet::mono() || l.getMainOutputChannelSet()==juce::AudioChannelSet::stereo());
}

void CardizOneAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {
  juce::ScopedNoDenormals noDenormals;
  if(dryBuffer.getNumChannels()!=b.getNumChannels() || dryBuffer.getNumSamples()<b.getNumSamples())
    dryBuffer.setSize(b.getNumChannels(),b.getNumSamples(),false,false,true);
  for(int c=0;c<b.getNumChannels();++c) dryBuffer.copyFrom(c,0,b,c,0,b.getNumSamples());
  auto in=b.getMagnitude(0,b.getNumSamples()); inputPeak.store(in);
  if(analysisRunning.load() && in > juce::Decibels::decibelsToGain(-55.f)) {
    auto coef=[this](float hz){ return std::exp(-2.f*juce::MathConstants<float>::pi*hz/(float)currentSampleRate); };
    const float a150=coef(150),a400=coef(400),a900=coef(900),a2500=coef(2500),a5000=coef(5000),a10000=coef(10000);
    for(int n=0;n<b.getNumSamples();++n) {
      float x=0; for(int c=0;c<b.getNumChannels();++c) x+=b.getSample(c,n); x/=juce::jmax(1,b.getNumChannels());
      analyzer150=a150*analyzer150+(1.f-a150)*x; analyzer400=a400*analyzer400+(1.f-a400)*x;
      analyzer900=a900*analyzer900+(1.f-a900)*x; analyzer2500=a2500*analyzer2500+(1.f-a2500)*x;
      analyzer5000=a5000*analyzer5000+(1.f-a5000)*x; analyzer10000=a10000*analyzer10000+(1.f-a10000)*x;
      const float lo=analyzer400, mi=analyzer2500-analyzer400, hi=x-analyzer2500;
      const float bands[] {analyzer400-analyzer150,analyzer900-analyzer400,analyzer2500-analyzer900,analyzer5000-analyzer2500};
      const float sib=analyzer10000-analyzer5000;
      sumSquares+=(double)x*x; lowSquares+=(double)lo*lo; midSquares+=(double)mi*mi; highSquares+=(double)hi*hi;
      for(int i=0;i<4;++i) resonanceSquares[i]+=(double)bands[i]*bands[i];
      sibilanceSquares+=(double)sib*sib;
      analysisPeakValue=juce::jmax(analysisPeakValue,std::abs(x)); analysisSamples++;
    }
    const double target=analysisTargetSamples;
    analysisProgress.store((float)juce::jlimit(0.0,1.0,analysisSamples/target));
    if(analysisSamples>=target) {
      const double total=juce::jmax(1.0,lowSquares+midSquares+highSquares);
      analysedRms.store(juce::Decibels::gainToDecibels((float)std::sqrt(sumSquares/analysisSamples),-60.f));
      analysedPeak.store(juce::Decibels::gainToDecibels(analysisPeakValue,-60.f));
      analysedLow.store((float)(lowSquares/total)); analysedMid.store((float)(midSquares/total)); analysedHigh.store((float)(highSquares/total));
      analysedCrest.store(analysedPeak.load()-analysedRms.load());
      analysedSibilance.store((float)(sibilanceSquares/total));
      int dominant=0; for(int i=1;i<4;++i) if(resonanceSquares[i]>resonanceSquares[dominant]) dominant=i;
      const double resonanceAverage=(resonanceSquares[0]+resonanceSquares[1]+resonanceSquares[2]+resonanceSquares[3])/4.0;
      const float centres[] {275.f,650.f,1600.f,3600.f};
      analysedResonanceHz.store(centres[dominant]);
      analysedResonanceSeverity.store((float)(resonanceSquares[dominant]/juce::jmax(1.0e-12,resonanceAverage)));
      analysisRunning.store(false); analysisReady.store(true); analysisProgress.store(1.f);
    }
  }
  if (apvts.getRawParameterValue("bypass")->load()>.5f) { outputPeak.store(in); return; }
  const float tonal=apvts.getRawParameterValue("tonal")->load()/100.f;
  const float glue=apvts.getRawParameterValue("glue")->load()/100.f;
  const float punch=apvts.getRawParameterValue("punch")->load()/100.f;
  const float wide=apvts.getRawParameterValue("width")->load()/100.f;
  const float deEss=apvts.getRawParameterValue("deEss")->load()/100.f;
  const float resonanceHz=apvts.getRawParameterValue("resonanceHz")->load();
  const float resonanceCut=apvts.getRawParameterValue("resonanceCut")->load();
  const int mode=(int)apvts.getRawParameterValue("mode")->load();
  const int style=(int)apvts.getRawParameterValue("voiceStyle")->load();
  const int activeStyle=mode==0 ? style : 3;
  const float stylePresence[] {1.8f,2.8f,.8f,.3f}, styleBody[] {-1.8f,-2.5f,1.2f,0.f}, styleAir[] {1.5f,3.0f,.6f,0.f};
  const float profile = mode==0 ? 1.f : (mode==1 ? .45f : 0.f);
  if(mode!=cachedMode || style!=cachedStyle || std::abs(tonal-cachedTonal)>.0001f) {
    lowCut.setCutoffFrequency(mode==0 ? 65.f : (mode==1 ? 24.f : 30.f));
    *presenceFilter.state=*juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate,3800.f,.85f,juce::Decibels::decibelsToGain((tonal-.5f)*5.f+stylePresence[activeStyle]*profile));
    *bodyFilter.state=*juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate,220.f,.8f,juce::Decibels::decibelsToGain(styleBody[activeStyle]*profile));
    *airFilter.state=*juce::dsp::IIR::Coefficients<float>::makeHighShelf(currentSampleRate,9000.f,.7071f,juce::Decibels::decibelsToGain(styleAir[activeStyle]*profile));
    cachedMode=mode; cachedStyle=style; cachedTonal=tonal;
  }
  if(std::abs(resonanceHz-cachedResonanceHz)>.01f || std::abs(resonanceCut-cachedResonanceCut)>.001f) {
    *resonanceFilter.state=*juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate,resonanceHz,3.2f,juce::Decibels::decibelsToGain(-resonanceCut));
    cachedResonanceHz=resonanceHz; cachedResonanceCut=resonanceCut;
  }
  compressor.setThreshold(-8.f-glue*16.f); compressor.setRatio(1.5f+glue*3.5f);
  compressor.setAttack(24.f-punch*20.f); compressor.setRelease(70.f+glue*160.f);
  limiter.setThreshold(apvts.getRawParameterValue("ceiling")->load()); limiter.setRelease(90.f);
  const float driveAmount=mode==0 ? .8f : (mode==1 ? .35f : .18f);
  drive.setTargetValue(1.f+driveAmount*glue); width.setTargetValue(.7f+wide*.8f);
  makeup.setTargetValue(juce::Decibels::decibelsToGain(apvts.getRawParameterValue("makeup")->load()));
  juce::dsp::AudioBlock<float> block(b); juce::dsp::ProcessContextReplacing<float> ctx(block);
  lowCut.process(ctx); bodyFilter.process(ctx); resonanceFilter.process(ctx); presenceFilter.process(ctx); airFilter.process(ctx);
  // Split-band dynamic de-esser: the high-frequency residual is reduced only
  // while its envelope crosses the sibilance threshold.
  const float deEssLpCoef=std::exp(-2.f*juce::MathConstants<float>::pi*5500.f/(float)currentSampleRate);
  const float attackCoef=std::exp(-1.f/(.0025f*(float)currentSampleRate));
  const float releaseCoef=std::exp(-1.f/(.075f*(float)currentSampleRate));
  for(int n=0;n<b.getNumSamples();++n) for(int c=0;c<b.getNumChannels();++c) {
    const float x=b.getSample(c,n);
    deEssLowpass[c]=deEssLpCoef*deEssLowpass[c]+(1.f-deEssLpCoef)*x;
    const float highBand=x-deEssLowpass[c], detector=std::abs(highBand);
    const float envCoef=detector>deEssEnvelope[c] ? attackCoef : releaseCoef;
    deEssEnvelope[c]=envCoef*deEssEnvelope[c]+(1.f-envCoef)*detector;
    const float activity=juce::jlimit(0.f,1.f,(deEssEnvelope[c]-.012f)/.055f);
    b.setSample(c,n,x-highBand*(activity*deEss*.72f));
  }
  compressor.process(ctx);
  for(int n=0;n<b.getNumSamples();++n) {
    const float d=drive.getNextValue();
    for(int c=0;c<b.getNumChannels();++c) b.setSample(c,n,std::tanh(b.getSample(c,n)*d));
  }
  if(b.getNumChannels()==2) for(int n=0;n<b.getNumSamples();++n) {
    float l=b.getSample(0,n),r=b.getSample(1,n),m=.5f*(l+r),s=.5f*(l-r)*width.getNextValue();
    b.setSample(0,n,m+s); b.setSample(1,n,m-s);
  }
  for(int n=0;n<b.getNumSamples();++n) {
    const float gain=makeup.getNextValue();
    for(int c=0;c<b.getNumChannels();++c) b.setSample(c,n,b.getSample(c,n)*gain);
  }
  limiter.process(ctx);
  float rms=0.f,dryRms=0.f;
  for(int c=0;c<b.getNumChannels();++c) { rms+=b.getRMSLevel(c,0,b.getNumSamples()); dryRms+=dryBuffer.getRMSLevel(c,0,b.getNumSamples()); }
  rms/=juce::jmax(1,b.getNumChannels()); dryRms/=juce::jmax(1,b.getNumChannels());
  const double blockSeconds=(double)b.getNumSamples()/juce::jmax(1.0,currentSampleRate);
  const double matchAlpha=std::exp(-blockSeconds/.4);
  wetMatchEnergy=matchAlpha*wetMatchEnergy+(1.0-matchAlpha)*(double)rms*rms;
  dryMatchEnergy=matchAlpha*dryMatchEnergy+(1.0-matchAlpha)*(double)dryRms*dryRms;
  const float matchedGain=(float)std::sqrt(wetMatchEnergy/juce::jmax(1.0e-12,dryMatchEnergy));
  abGain.setTargetValue(juce::jlimit(.25f,4.f,matchedGain));
  comparisonMix.setTargetValue(apvts.getRawParameterValue("compareDry")->load()>.5f ? 1.f : 0.f);
  for(int n=0;n<b.getNumSamples();++n) {
    const float dryGain=abGain.getNextValue();
    const float mix=comparisonMix.getNextValue();
    for(int c=0;c<b.getNumChannels();++c) {
      const float wet=b.getSample(c,n), matchedDry=dryBuffer.getSample(c,n)*dryGain;
      b.setSample(c,n,wet+(matchedDry-wet)*mix);
    }
  }
  float out=b.getMagnitude(0,b.getNumSamples()); outputPeak.store(out);
  const double alpha=std::exp(-blockSeconds/0.4);
  momentaryEnergy=alpha*momentaryEnergy+(1.0-alpha)*(double)rms*rms;
  loudness.store(juce::jlimit(-60.f,0.f,juce::Decibels::gainToDecibels((float)std::sqrt(momentaryEnergy),-60.f)-.7f));
}

void CardizOneAudioProcessor::getStateInformation(juce::MemoryBlock& d) { if(auto x=apvts.copyState().createXml()) copyXmlToBinary(*x,d); }
void CardizOneAudioProcessor::setStateInformation(const void* d,int s) { if(auto x=getXmlFromBinary(d,s)) apvts.replaceState(juce::ValueTree::fromXml(*x)); }
juce::AudioProcessorEditor* CardizOneAudioProcessor::createEditor(){ return new CardizOneAudioProcessorEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){ return new CardizOneAudioProcessor(); }

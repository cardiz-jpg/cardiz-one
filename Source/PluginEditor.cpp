#include "PluginEditor.h"
static const auto gold=juce::Colour::fromRGB(224,178,86), ivory=juce::Colour::fromRGB(242,235,217);

GoldLookAndFeel::GoldLookAndFeel(){ setColour(juce::Slider::textBoxTextColourId,gold); setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack); }
void GoldLookAndFeel::drawRotarySlider(juce::Graphics& g,int x,int y,int w,int h,float p,float a0,float a1,juce::Slider&){
  auto r=juce::Rectangle<float>((float)x,(float)y,(float)w,(float)h).reduced(10); auto c=r.getCentre();
  g.setColour(juce::Colour::fromRGB(7,8,9)); g.fillEllipse(r); g.setColour(gold.darker(.45f)); g.drawEllipse(r,2.f);
  juce::Path arc; arc.addCentredArc(c.x,c.y,r.getWidth()*.55f,r.getHeight()*.55f,0,a0,a0+(a1-a0)*p,true); g.setColour(gold); g.strokePath(arc,juce::PathStrokeType(3.f));
  juce::Path needle; needle.addRoundedRectangle(-1.5f,-r.getHeight()*.30f,3.f,r.getHeight()*.27f,1.5f); g.fillPath(needle,juce::AffineTransform::rotation(a0+(a1-a0)*p).translated(c.x,c.y));
}
void GoldLookAndFeel::drawButtonBackground(juce::Graphics& g,juce::Button& b,const juce::Colour&,bool over,bool down){
  g.setColour(down?gold.withAlpha(.3f):(over?gold.withAlpha(.16f):juce::Colour::fromRGB(14,16,17))); g.fillRoundedRectangle(b.getLocalBounds().toFloat(),5.f); g.setColour(gold.withAlpha(.9f)); g.drawRoundedRectangle(b.getLocalBounds().toFloat().reduced(.5f),5.f,1.f);
}

CardizOneAudioProcessorEditor::CardizOneAudioProcessorEditor(CardizOneAudioProcessor& p):AudioProcessorEditor(&p),processor(p){
  setLookAndFeel(&look); setResizable(true,true); setResizeLimits(760,430,1400,800); setSize(1000,570);
  for(auto* s:{&tonal,&glue,&punch,&width}){ addAndMakeVisible(s); s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); s->setTextBoxStyle(juce::Slider::TextBoxBelow,false,55,20); }
  for(auto* b:{&vox,&master,&pro,&finish}){ addAndMakeVisible(b); b->setColour(juce::TextButton::textColourOffId,ivory); }
  vox.onClick=[this]{*processor.apvts.getRawParameterValue("mode")=0;}; master.onClick=[this]{*processor.apvts.getRawParameterValue("mode")=1;}; pro.onClick=[this]{*processor.apvts.getRawParameterValue("mode")=2;};
  finish.onClick=[this]{ auto* a=processor.apvts.getRawParameterValue("analyze"); *a=a->load()>.5f?0.f:1.f; };
  aTonal=std::make_unique<SA>(p.apvts,"tonal",tonal); aGlue=std::make_unique<SA>(p.apvts,"glue",glue); aPunch=std::make_unique<SA>(p.apvts,"punch",punch); aWidth=std::make_unique<SA>(p.apvts,"width",width); startTimerHz(30);
}
CardizOneAudioProcessorEditor::~CardizOneAudioProcessorEditor(){ setLookAndFeel(nullptr); }
void CardizOneAudioProcessorEditor::paint(juce::Graphics& g){
  g.fillAll(juce::Colour::fromRGB(8,10,11)); auto b=getLocalBounds().toFloat().reduced(10); g.setColour(gold); g.drawRoundedRectangle(b,8,1.2f);
  g.setColour(ivory); g.setFont(juce::FontOptions(34)); g.drawFittedText("C A R D I Z  O N E",getLocalBounds().removeFromTop(72),juce::Justification::centredBottom,1);
  g.setFont(juce::FontOptions(11)); g.drawFittedText("F R O M  V O I C E  T O  M A S T E R",0,74,getWidth(),18,juce::Justification::centred,1);
  auto cy=getHeight()/2+15,cx=getWidth()/2; float rad=80; g.setColour(gold.withAlpha(.14f)); g.fillEllipse(cx-rad-10,cy-rad-10,rad*2+20,rad*2+20); g.setColour(juce::Colour::fromRGB(5,6,7)); g.fillEllipse(cx-rad,cy-rad,rad*2,rad*2); g.setColour(gold); g.drawEllipse(cx-rad,cy-rad,rad*2,rad*2,7);
  auto l=processor.loudness.load(); g.setColour(gold.brighter(.2f)); g.setFont(juce::FontOptions(42)); g.drawText(juce::String(l,0),cx-70,cy-48,140,55,juce::Justification::centred); g.setColour(ivory); g.setFont(juce::FontOptions(14)); g.drawText("LUFS",cx-60,cy+5,120,20,juce::Justification::centred); g.setFont(juce::FontOptions(11)); g.drawText("MASTER READY",cx-75,cy+26,150,20,juce::Justification::centred);
  g.setFont(juce::FontOptions(13)); for(auto pair:{std::pair<juce::Slider*,juce::String>{&tonal,"TONAL BALANCE"},{&glue,"GLUE"},{&punch,"PUNCH"},{&width,"WIDTH"}}) g.drawText(pair.second,pair.first->getX()-10,pair.first->getY()-25,pair.first->getWidth()+20,20,juce::Justification::centred);
  auto meter=[&](int x,float v,juce::String label){g.setColour(ivory);g.setFont(juce::FontOptions(11));g.drawText(label,x,515,65,20,juce::Justification::left);g.setColour(juce::Colour::fromRGB(52,54,54));g.fillRect(x+70,522,150,5);g.setColour(gold);g.fillRect(x+70,522,(int)(150*juce::jlimit(0.f,1.f,v)),5);}; meter(35,processor.inputPeak.load(),"INPUT");meter(getWidth()-255,processor.outputPeak.load(),"OUTPUT");
  g.setColour(ivory.withAlpha(.7f));g.setFont(juce::FontOptions(11));g.drawText("MASTERING ENHANCEMENT",cx-130,500,260,20,juce::Justification::centred);g.drawText("NATURAL  ·  MUSICAL  ·  PROFESSIONAL",cx-165,525,330,20,juce::Justification::centred);
}
void CardizOneAudioProcessorEditor::resized(){
  int cx=getWidth()/2; vox.setBounds(cx-235,110,145,35);master.setBounds(cx-72,110,145,35);pro.setBounds(cx+90,110,145,35);
  int y=210,s=125; tonal.setBounds(55,y,s,s);glue.setBounds(230,y,s,s);punch.setBounds(getWidth()-355,y,s,s);width.setBounds(getWidth()-180,y,s,s);finish.setBounds(cx-85,405,170,40);
}
void CardizOneAudioProcessorEditor::timerCallback(){ repaint(); }

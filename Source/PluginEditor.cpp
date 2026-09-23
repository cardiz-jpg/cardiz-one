#include "PluginEditor.h"
#include <cmath>

namespace C {
const auto black = juce::Colour::fromRGB(4, 5, 6);
const auto carbon = juce::Colour::fromRGB(12, 14, 15);
const auto panel = juce::Colour::fromRGB(19, 20, 20);
const auto gold = juce::Colour::fromRGB(224, 178, 86);
const auto pale = juce::Colour::fromRGB(255, 226, 162);
const auto ivory = juce::Colour::fromRGB(244, 238, 222);
const auto muted = juce::Colour::fromRGB(137, 134, 125);
}

static juce::ColourGradient vGradient(juce::Colour top, juce::Colour bottom, juce::Rectangle<float> r) {
  return { top, r.getCentreX(), r.getY(), bottom, r.getCentreX(), r.getBottom(), false };
}

GoldLookAndFeel::GoldLookAndFeel() {
  setColour(juce::Slider::textBoxTextColourId, C::pale);
  setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
  setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

juce::Font GoldLookAndFeel::getTextButtonFont(juce::TextButton&, int h) {
  return juce::Font(juce::FontOptions(juce::jlimit(11.0f, 14.0f, h * 0.34f)))
      .withExtraKerningFactor(0.08f);
}

void GoldLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                        float pos, float start, float end, juce::Slider&) {
  auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) w, (float) h).reduced(14.0f);
  const auto d = juce::jmin(bounds.getWidth(), bounds.getHeight());
  auto dial = juce::Rectangle<float>(d, d).withCentre(bounds.getCentre());
  auto centre = dial.getCentre();
  const auto angle = start + pos * (end - start);

  for (int i = 4; i > 0; --i) {
    g.setColour(C::gold.withAlpha(0.022f * (5 - i)));
    g.fillEllipse(dial.expanded(i * 4.0f));
  }
  for (int i = 0; i <= 28; ++i) {
    const auto a = start + (end - start) * ((float) i / 28.0f);
    const auto inner = centre + juce::Point<float>(std::sin(a), -std::cos(a)) * (d * 0.54f);
    const auto outer = centre + juce::Point<float>(std::sin(a), -std::cos(a))
                                  * (d * (i % 7 == 0 ? 0.605f : 0.58f));
    g.setColour(i / 28.0f <= pos ? C::pale : C::gold.withAlpha(0.28f));
    g.drawLine({inner, outer}, i % 7 == 0 ? 1.4f : 0.75f);
  }

  g.setGradientFill(vGradient(juce::Colour::fromRGB(43, 43, 40), C::black, dial));
  g.fillEllipse(dial);
  g.setColour(C::gold.darker(0.46f));
  g.drawEllipse(dial, 2.0f);
  g.setColour(C::pale.withAlpha(0.24f));
  g.drawEllipse(dial.reduced(4.0f), 0.9f);

  juce::Path arc;
  arc.addCentredArc(centre.x, centre.y, d * 0.51f, d * 0.51f, 0.0f, start, angle, true);
  g.setColour(C::pale.withAlpha(0.95f));
  g.strokePath(arc, juce::PathStrokeType(2.5f, juce::PathStrokeType::curved,
                                         juce::PathStrokeType::rounded));

  juce::Path needle;
  needle.addRoundedRectangle(-1.6f, -d * 0.36f, 3.2f, d * 0.29f, 1.6f);
  g.setColour(C::ivory);
  g.fillPath(needle, juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
  g.setColour(C::gold.darker(0.30f));
  g.fillEllipse(juce::Rectangle<float>(8.0f, 8.0f).withCentre(centre));
}

void GoldLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                            const juce::Colour&, bool over, bool down) {
  auto r = b.getLocalBounds().toFloat().reduced(0.7f);
  const bool active = b.getToggleState();
  auto top = active ? juce::Colour::fromRGB(58, 47, 27) : C::panel;
  if (over) top = top.brighter(0.08f);
  if (down) top = top.darker(0.10f);
  g.setGradientFill(vGradient(top, C::black, r));
  g.fillRoundedRectangle(r, 5.0f);
  g.setColour(active ? C::pale : C::gold.withAlpha(0.43f));
  g.drawRoundedRectangle(r, 5.0f, active ? 1.45f : 0.8f);
  if (active) {
    g.setColour(C::gold.withAlpha(0.07f));
    g.fillRoundedRectangle(r.reduced(3.0f), 3.0f);
  }
}

void GoldLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool, bool) {
  g.setFont(getTextButtonFont(b, b.getHeight()));
  g.setColour(b.getToggleState() ? C::pale : C::ivory.withAlpha(0.93f));
  g.drawFittedText(b.getButtonText(), b.getLocalBounds().reduced(8, 2),
                   juce::Justification::centred, 1);
}

CardizOneAudioProcessorEditor::CardizOneAudioProcessorEditor(CardizOneAudioProcessor& p)
  : AudioProcessorEditor(&p), processor(p) {
  setLookAndFeel(&look);
  setResizable(false, false);
  setSize(1000, 650);

  for (auto* s : {&tonal, &glue, &punch, &width}) {
    addAndMakeVisible(s);
    s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 62, 19);
  }
  for (auto* b : {&vox, &master, &pro, &finish, &before, &after}) {
    addAndMakeVisible(b);
    b->setClickingTogglesState(false);
  }
  addAndMakeVisible(voiceStyle);
  voiceStyle.addItemList({"LATINO URBANO","ELECTRONICA","BALADA","NATURAL"},1);
  addAndMakeVisible(masterStyle);
  masterStyle.addItemList({"STREAMING MODERNO","CLUB / ELECTRONICA","BALADA / ORGANICO","TRANSPARENTE"},1);
  addAndMakeVisible(proStyle);
  proStyle.addItemList({"IMPACTO CONTROLADO","DINAMICA ABIERTA","BALANCE CALIDO","REFERENCIA NEUTRA"},1);
  for(auto* profile:{&voiceStyle,&masterStyle,&proStyle}) {
    profile->setJustificationType(juce::Justification::centred);
    profile->setColour(juce::ComboBox::backgroundColourId,C::black);
    profile->setColour(juce::ComboBox::outlineColourId,C::gold.withAlpha(.65f));
    profile->setColour(juce::ComboBox::textColourId,C::ivory);
  }
  addAndMakeVisible(analysisStatus);
  analysisStatus.setJustificationType(juce::Justification::centred);
  analysisStatus.setColour(juce::Label::textColourId,C::pale);
  analysisStatus.setFont(juce::Font(juce::FontOptions(10.f)).withExtraKerningFactor(.08f));
  for(auto* label:{&diagnosticDetail,&recommendation}) {
    addAndMakeVisible(label);
    label->setJustificationType(juce::Justification::centred);
    label->setColour(juce::Label::textColourId,C::ivory.withAlpha(.86f));
    label->setFont(juce::Font(juce::FontOptions(10.f)).withExtraKerningFactor(.04f));
    label->setMinimumHorizontalScale(.70f);
  }
  diagnosticDetail.setText("RMS · CREST · SIBILANCIA · RESONANCIAS · BALANCE TONAL",juce::dontSendNotification);
  recommendation.setText("Selecciona un modo y reproduce una zona representativa de la pista",juce::dontSendNotification);

  vox.onClick = [this] { selectMode(0, true); };
  master.onClick = [this] { selectMode(1, true); };
  pro.onClick = [this] { selectMode(2, true); };
  finish.onClick = [this] {
    processor.beginAnalysis();
    analysisStatus.setText("ESCUCHANDO LA PISTA...",juce::dontSendNotification);
    diagnosticDetail.setText("Midiendo energia, dinamica y contenido espectral",juce::dontSendNotification);
    recommendation.setText("Mantén la reproduccion activa durante todo el analisis",juce::dontSendNotification);
  };
  before.onClick=[this] {
    if(auto* p=processor.apvts.getParameter("compareDry")) p->setValueNotifyingHost(1.f);
  };
  after.onClick=[this] {
    if(auto* p=processor.apvts.getParameter("compareDry")) p->setValueNotifyingHost(0.f);
  };

  aTonal = std::make_unique<SA>(p.apvts, "tonal", tonal);
  aGlue = std::make_unique<SA>(p.apvts, "glue", glue);
  aPunch = std::make_unique<SA>(p.apvts, "punch", punch);
  aWidth = std::make_unique<SA>(p.apvts, "width", width);
  aStyle = std::make_unique<CA>(p.apvts,"voiceStyle",voiceStyle);
  aMasterStyle = std::make_unique<CA>(p.apvts,"masterStyle",masterStyle);
  aProStyle = std::make_unique<CA>(p.apvts,"proStyle",proStyle);
  selectMode((int) p.apvts.getRawParameterValue("mode")->load());
  startTimerHz(30);
}

CardizOneAudioProcessorEditor::~CardizOneAudioProcessorEditor() { setLookAndFeel(nullptr); }

void CardizOneAudioProcessorEditor::selectMode(int mode, bool applyPreset) {
  if(auto* p=processor.apvts.getParameter("mode"))
    p->setValueNotifyingHost(p->convertTo0to1((float)mode));
  processor.analysisRunning.store(false);
  processor.analysisReady.store(false);
  processor.analysisProgress.store(0.f);
  vox.setToggleState(mode == 0, juce::dontSendNotification);
  master.setToggleState(mode == 1, juce::dontSendNotification);
  pro.setToggleState(mode == 2, juce::dontSendNotification);
  voiceStyle.setVisible(mode==0);
  masterStyle.setVisible(mode==1);
  proStyle.setVisible(mode==2);
  finish.setButtonText(mode==0 ? "ANALIZAR VOZ" : (mode==1 ? "ANALIZAR MASTER" : "ANALIZAR"));
  if(applyPreset && mode==1) {
    struct V { const char* id; float value; } values[] {{"tonal",55.f},{"glue",52.f},{"punch",60.f},{"width",55.f},{"makeup",1.5f}};
    for(auto v:values) if(auto* p=processor.apvts.getParameter(v.id)) p->setValueNotifyingHost(p->convertTo0to1(v.value));
    analysisStatus.setText("ONE MASTER · ELIGE PERFIL Y ANALIZA LA MEZCLA",juce::dontSendNotification);
  } else if(applyPreset && mode==0) {
    analysisStatus.setText("ESCOGE UN ESTILO Y PULSA ANALIZAR VOZ",juce::dontSendNotification);
  } else if(applyPreset) {
    analysisStatus.setText("PRO · ELIGE ESTRATEGIA Y EJECUTA EL DIAGNOSTICO",juce::dontSendNotification);
  }
  if(applyPreset) {
    diagnosticDetail.setText("RMS · CREST · SIBILANCIA · RESONANCIAS · BALANCE TONAL",juce::dontSendNotification);
    recommendation.setText("Reproduce una zona representativa antes de iniciar el analisis",juce::dontSendNotification);
  }
  repaint();
}

void CardizOneAudioProcessorEditor::drawMeter(juce::Graphics& g, juce::Rectangle<float> area,
                                               float value, const juce::String& label) {
  g.setColour(C::ivory.withAlpha(0.93f));
  g.setFont(juce::Font(juce::FontOptions(11.0f)).withExtraKerningFactor(0.08f));
  g.drawText(label, area.removeFromLeft(58.0f), juce::Justification::centredLeft);
  auto bar = area.withSizeKeepingCentre(area.getWidth(), 7.0f);
  g.setColour(juce::Colour::fromRGB(48, 49, 48));
  g.fillRoundedRectangle(bar, 2.0f);
  auto fill = bar.withWidth(bar.getWidth() * juce::jlimit(0.0f, 1.0f, value));
  juce::ColourGradient glow(C::gold, fill.getX(), fill.getCentreY(), C::pale,
                             fill.getRight(), fill.getCentreY(), false);
  g.setGradientFill(glow);
  g.fillRoundedRectangle(fill, 2.0f);
  g.setColour(C::pale.withAlpha(0.18f));
  g.fillRoundedRectangle(fill.expanded(0.0f, 2.0f), 3.0f);

  g.setColour(C::muted);
  g.setFont(juce::FontOptions(8.5f));
  const juce::StringArray marks {"-60", "-36", "-24", "-12", "-6", "0"};
  for (int i = 0; i < marks.size(); ++i) {
    auto px = bar.getX() + bar.getWidth() * ((float) i / (marks.size() - 1));
    g.drawText(marks[i], (int) px - 13, (int) bar.getBottom() + 4, 26, 12,
               juce::Justification::centred);
  }
}

void CardizOneAudioProcessorEditor::drawLufsDial(juce::Graphics& g,
                                                  juce::Rectangle<float> area) {
  auto centre = area.getCentre();
  auto d = juce::jmin(area.getWidth(), area.getHeight());
  auto ring = juce::Rectangle<float>(d, d).withCentre(centre).reduced(5.0f);
  for (int i = 5; i > 0; --i) {
    g.setColour(C::gold.withAlpha(0.018f * (6 - i)));
    g.fillEllipse(ring.expanded(i * 4.0f));
  }
  g.setGradientFill(vGradient(juce::Colour::fromRGB(28, 27, 23), C::black, ring));
  g.fillEllipse(ring);
  g.setColour(C::gold.darker(0.45f));
  g.drawEllipse(ring, 2.0f);

  const auto lufs = processor.loudness.load();
  const auto norm = juce::jlimit(0.0f, 1.0f, (lufs + 60.0f) / 60.0f);
  constexpr float begin = -2.45f, end = 2.45f;
  for (int i = 0; i <= 36; ++i) {
    const auto a = begin + (end - begin) * ((float) i / 36.0f);
    auto p1 = centre + juce::Point<float>(std::sin(a), -std::cos(a)) * (d * 0.415f);
    auto p2 = centre + juce::Point<float>(std::sin(a), -std::cos(a))
                           * (d * (i % 6 == 0 ? 0.475f : 0.455f));
    g.setColour(i / 36.0f <= norm ? C::pale : C::gold.withAlpha(0.20f));
    g.drawLine({p1, p2}, i % 6 == 0 ? 1.55f : 0.75f);
  }
  juce::Path arc;
  arc.addCentredArc(centre.x, centre.y, d * 0.45f, d * 0.45f, 0.0f, begin,
                    begin + (end - begin) * norm, true);
  g.setColour(C::pale);
  g.strokePath(arc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved,
                                         juce::PathStrokeType::rounded));

  g.setColour(C::pale);
  g.setFont(juce::Font(juce::FontOptions(d * 0.23f)));
  g.drawText(juce::String(lufs, 0), ring.withTrimmedTop(d * 0.25f).withHeight(d * 0.28f),
             juce::Justification::centred);
  g.setColour(C::ivory);
  g.setFont(juce::Font(juce::FontOptions(d * 0.072f)).withExtraKerningFactor(0.12f));
  g.drawText("LUFS-M EST.", ring.withTrimmedTop(d * 0.51f).withHeight(d * 0.10f),
             juce::Justification::centred);
  g.setFont(juce::Font(juce::FontOptions(d * 0.057f)).withExtraKerningFactor(0.06f));
  g.drawText(processor.analysisRunning.load() ? "ANALYZING" : "SMART ENGINE", ring.withTrimmedTop(d * 0.62f).withHeight(d * 0.10f),
             juce::Justification::centred);
  g.setColour(C::muted);
  g.setFont(juce::FontOptions(d * 0.043f));
  g.drawText("OUTPUT MOMENTARY", ring.withTrimmedTop(d * 0.73f).withHeight(d * 0.08f),
             juce::Justification::centred);
}

void CardizOneAudioProcessorEditor::paint(juce::Graphics& g) {
  auto all = getLocalBounds().toFloat();
  g.setGradientFill(vGradient(juce::Colour::fromRGB(23, 25, 25), C::black, all));
  g.fillRect(all);

  // Carbon weave matching the close-up material used in the cinematic.
  g.setColour(juce::Colours::white.withAlpha(0.012f));
  for (int x = -getHeight(); x < getWidth(); x += 14)
    g.drawLine((float) x, 0.0f, (float) x + getHeight(), (float) getHeight(), 0.55f);
  g.setColour(juce::Colours::black.withAlpha(0.11f));
  for (int x = 0; x < getWidth() + getHeight(); x += 14)
    g.drawLine((float) x, 0.0f, (float) x - getHeight(), (float) getHeight(), 0.55f);

  auto outer = all.reduced(10.0f);
  g.setColour(C::gold.withAlpha(0.72f));
  g.drawRoundedRectangle(outer, 8.0f, 1.0f);
  g.setColour(C::pale.withAlpha(0.10f));
  g.drawRoundedRectangle(outer.reduced(3.0f), 6.0f, 0.7f);

  g.setColour(C::ivory);
  g.setFont(juce::Font(juce::FontOptions(34.0f)).withExtraKerningFactor(0.24f));
  g.drawFittedText("CARDIZ ONE", 0, 27, getWidth(), 45, juce::Justification::centred, 1);
  g.setColour(C::pale.withAlpha(0.95f));
  g.setFont(juce::Font(juce::FontOptions(10.5f)).withExtraKerningFactor(0.30f));
  g.drawText("FROM VOICE TO MASTER", 0, 75, getWidth(), 18, juce::Justification::centred);

  auto panel = juce::Rectangle<float>(26.0f, 108.0f, all.getWidth() - 52.0f,
                                      365.0f);
  g.setGradientFill(vGradient(C::panel.withAlpha(0.86f), C::black.withAlpha(0.93f), panel));
  g.fillRoundedRectangle(panel, 7.0f);
  g.setColour(C::gold.withAlpha(0.56f));
  g.drawRoundedRectangle(panel, 7.0f, 0.9f);
  g.setColour(C::pale.withAlpha(0.055f));
  g.drawHorizontalLine((int) panel.getY() + 63, panel.getX() + 16, panel.getRight() - 16);
  {
    const int mode=(int)processor.apvts.getRawParameterValue("mode")->load();
    g.setColour(C::muted);
    g.setFont(juce::Font(juce::FontOptions(9.f)).withExtraKerningFactor(.15f));
    const juce::String label=mode==0 ? "PERFIL VOCAL" : (mode==1 ? "PERFIL MASTER" : "ESTRATEGIA PRO");
    g.drawText(label,280,169,103,18,juce::Justification::centredRight);
  }

  g.setColour(C::ivory.withAlpha(0.96f));
  g.setFont(juce::Font(juce::FontOptions(12.3f)).withExtraKerningFactor(0.06f));
  const std::pair<juce::Slider*, juce::String> labels[] = {
    {&tonal, "TONAL BALANCE"}, {&glue, "GLUE"}, {&punch, "PUNCH"}, {&width, "WIDTH"}
  };
  for (const auto& item : labels)
    g.drawText(item.second, item.first->getX() - 15, item.first->getY() - 24,
               item.first->getWidth() + 30, 18, juce::Justification::centred);

  drawLufsDial(g, {(float) getWidth() / 2.0f - 91.0f, 196.0f, 182.0f, 182.0f});
  g.setColour(C::pale.withAlpha(0.92f));
  g.setFont(juce::Font(juce::FontOptions(9.5f)).withExtraKerningFactor(0.22f));
  g.drawText("CLEAN  ·  BALANCED  ·  POWERFUL", getWidth() / 2 - 180, 378, 360, 18,
             juce::Justification::centred);

  auto report=juce::Rectangle<float>(26.f,485.f,all.getWidth()-52.f,145.f);
  g.setGradientFill(vGradient(juce::Colour::fromRGB(16,17,17),C::black,report));
  g.fillRoundedRectangle(report,7.f);
  g.setColour(C::gold.withAlpha(.42f));
  g.drawRoundedRectangle(report,7.f,.8f);
  g.setColour(C::pale.withAlpha(.78f));
  g.setFont(juce::Font(juce::FontOptions(9.5f)).withExtraKerningFactor(.24f));
  g.drawText("DIAGNOSTICO INTELIGENTE",350,490,300,16,juce::Justification::centred);

  drawMeter(g, {35.0f, (float) getHeight() - 61.0f, 255.0f, 25.0f},
            processor.inputPeak.load(), "INPUT");
  drawMeter(g, {(float) getWidth() - 290.0f, (float) getHeight() - 61.0f, 255.0f, 25.0f},
            processor.outputPeak.load(), "OUTPUT");
}

void CardizOneAudioProcessorEditor::resized() {
  vox.setBounds(270, 121, 145, 34);
  master.setBounds(428, 121, 145, 34);
  pro.setBounds(586, 121, 145, 34);
  voiceStyle.setBounds(390,164,220,30);
  masterStyle.setBounds(390,164,220,30);
  proStyle.setBounds(390,164,220,30);
  tonal.setBounds(57, 232, 132, 142);
  glue.setBounds(221, 232, 132, 142);
  punch.setBounds(647, 232, 132, 142);
  width.setBounds(811, 232, 132, 142);
  finish.setBounds(416, 414, 168, 39);
  analysisStatus.setBounds(235,452,530,24);
  diagnosticDetail.setBounds(190,508,620,20);
  recommendation.setBounds(145,530,710,20);
  before.setBounds(355,556,140,28);
  after.setBounds(505,556,140,28);
}

void CardizOneAudioProcessorEditor::timerCallback() {
  if(processor.analysisRunning.load()) {
    const auto pc=(int)(processor.analysisProgress.load()*100.f);
    analysisStatus.setText(pc==0 ? "REPRODUCE LA PISTA PARA COMENZAR" : "ANALIZANDO  "+juce::String(pc)+"%",juce::dontSendNotification);
  } else if(processor.applyAnalysisResult()) {
    const int mode=(int)processor.apvts.getRawParameterValue("mode")->load();
    const int selectedProfile=(int)processor.apvts.getRawParameterValue(mode==0 ? "voiceStyle" : (mode==1 ? "masterStyle" : "proStyle"))->load();
    const juce::StringArray voiceProfiles {"LATINO URBANO","ELECTRONICA","BALADA","NATURAL"};
    const juce::StringArray masterProfiles {"STREAMING MODERNO","CLUB / ELECTRONICA","BALADA / ORGANICO","TRANSPARENTE"};
    const juce::StringArray proProfiles {"IMPACTO CONTROLADO","DINAMICA ABIERTA","BALANCE CALIDO","REFERENCIA NEUTRA"};
    const juce::String profileName=mode==0 ? voiceProfiles[selectedProfile] : (mode==1 ? masterProfiles[selectedProfile] : proProfiles[selectedProfile]);
    const auto low=processor.analysedLow.load(), high=processor.analysedHigh.load();
    const juce::String tone=low>.48f ? "VOZ OSCURA" : (high>.25f ? "VOZ BRILLANTE" : "VOZ EQUILIBRADA");
    const juce::String balance=low>.48f ? "BALANCE GRAVE" : (high>.25f ? "BALANCE BRILLANTE" : "BALANCE NEUTRO");
    const juce::String resultTone=mode==0 ? tone : balance;
    const juce::String resultMode=mode==0 ? "VOZ" : (mode==1 ? "MASTER" : "PRO");
    analysisStatus.setText(resultMode+" / "+profileName+" · RMS "+juce::String(processor.analysedRms.load(),1)+" dB · "+resultTone+" · APLICADO",juce::dontSendNotification);
    const bool sibilant=processor.analysedSibilance.load()>.025f;
    const bool resonant=processor.analysedResonanceSeverity.load()>1.25f;
    diagnosticDetail.setText("CREST "+juce::String(processor.analysedCrest.load(),1)+" dB  ·  SIBILANCIA "+(sibilant?"ALTA":"CONTROLADA")+"  ·  RESONANCIA "+juce::String(processor.analysedResonanceHz.load(),0)+" Hz",juce::dontSendNotification);
    juce::String advice="Nivel y balance optimizados";
    if(sibilant && resonant) advice="De-esser y reduccion de resonancia aplicados automaticamente";
    else if(sibilant) advice="Correccion de sibilancia aplicada; revisa las consonantes S y CH";
    else if(resonant) advice="Resonancia dominante atenuada sin vaciar el cuerpo de la pista";
    recommendation.setText("RECOMENDACION · "+advice+" · compara ANTES / CARDIZ ONE",juce::dontSendNotification);
  }
  const int mode = (int) processor.apvts.getRawParameterValue("mode")->load();
  vox.setToggleState(mode == 0, juce::dontSendNotification);
  master.setToggleState(mode == 1, juce::dontSendNotification);
  pro.setToggleState(mode == 2, juce::dontSendNotification);
  finish.setToggleState(processor.analysisRunning.load(),juce::dontSendNotification);
  const bool dry=processor.apvts.getRawParameterValue("compareDry")->load()>.5f;
  before.setToggleState(dry,juce::dontSendNotification);
  after.setToggleState(!dry,juce::dontSendNotification);
  repaint();
}

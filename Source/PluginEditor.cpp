#include "PluginEditor.h"

//==============================================================================
NovemberResonanceAudioProcessorEditor::NovemberResonanceAudioProcessorEditor (NovemberResonanceAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (720, 420);

    auto setupSlider = [this] (juce::Slider& s, juce::Label& l, const juce::String& name, const juce::String& paramId)
    {
        addAndMakeVisible (s);
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 20);
        s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xFFE8A838));
        s.setColour (juce::Slider::thumbColourId, juce::Colour (0xFFE8A838));
        s.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
        s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

        addAndMakeVisible (l);
        l.setText (name, juce::dontSendNotification);
        l.setJustificationType (juce::Justification::centred);
        l.setColour (juce::Label::textColourId, juce::Colour (0xFFE8A838));
        l.attachToComponent (&s, false);
    };

    setupSlider (cutoffSlider, cutoffLabel, "Cutoff", "cutoff");
    setupSlider (resonanceSlider, resonanceLabel, "Resonance", "resonance");
    setupSlider (attackSlider, attackLabel, "Attack", "attack");
    setupSlider (decaySlider, decayLabel, "Decay", "decay");
    setupSlider (sustainSlider, sustainLabel, "Sustain", "sustain");
    setupSlider (releaseSlider, releaseLabel, "Release", "release");
    setupSlider (mixSlider, mixLabel, "Osc Mix", "mix");
    setupSlider (atSlider, atLabel, "AT Amount", "atAmount");
    setupSlider (vibSlider, vibLabel, "Vibrato", "vibrato");

    auto& apvts = processor.getAPVTS();
    cutoffAttach = std::make_unique<Attachment> (apvts, "cutoff", cutoffSlider);
    resAttach = std::make_unique<Attachment> (apvts, "resonance", resonanceSlider);
    attAttach = std::make_unique<Attachment> (apvts, "attack", attackSlider);
    decAttach = std::make_unique<Attachment> (apvts, "decay", decaySlider);
    susAttach = std::make_unique<Attachment> (apvts, "sustain", sustainSlider);
    relAttach = std::make_unique<Attachment> (apvts, "release", releaseSlider);
    mixAttach = std::make_unique<Attachment> (apvts, "mix", mixSlider);
    atAttach = std::make_unique<Attachment> (apvts, "atAmount", atSlider);
    vibAttach = std::make_unique<Attachment> (apvts, "vibrato", vibSlider);
}

NovemberResonanceAudioProcessorEditor::~NovemberResonanceAudioProcessorEditor() = default;

//==============================================================================
void NovemberResonanceAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF0A0A12)); // deep charcoal

    auto bounds = getLocalBounds();
    g.setColour (juce::Colour (0xFFE8A838));
    g.setFont (juce::FontOptions (28.0f, juce::Font::bold));
    g.drawText ("November Resonance", bounds.removeFromTop (50), juce::Justification::centred);

    g.setFont (juce::FontOptions (14.0f));
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.drawText ("Cinematic Pad • Aftertouch + Sustain Pedal", bounds.removeFromTop (20), juce::Justification::centred);
}

void NovemberResonanceAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20).removeFromBottom (getHeight() - 80);
    auto row1 = area.removeFromTop (area.getHeight() / 2);
    auto row2 = area;

    const int w = 80;
    const int gap = 10;

    cutoffSlider.setBounds (row1.removeFromLeft (w)); row1.removeFromLeft (gap);
    resonanceSlider.setBounds (row1.removeFromLeft (w)); row1.removeFromLeft (gap);
    attackSlider.setBounds (row1.removeFromLeft (w)); row1.removeFromLeft (gap);
    decaySlider.setBounds (row1.removeFromLeft (w)); row1.removeFromLeft (gap);
    sustainSlider.setBounds (row1.removeFromLeft (w));

    releaseSlider.setBounds (row2.removeFromLeft (w)); row2.removeFromLeft (gap);
    mixSlider.setBounds (row2.removeFromLeft (w)); row2.removeFromLeft (gap);
    atSlider.setBounds (row2.removeFromLeft (w)); row2.removeFromLeft (gap);
    vibSlider.setBounds (row2.removeFromLeft (w));
}

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

//==============================================================================
/** Dark cinematic UI notes:
    - Background: deep charcoal #0A0A12
    - Accent: warm amber #E8A838 for labels and knob highlights
    - Secondary: muted teal #3A6A7A for secondary elements
    - Subtle drop shadows and soft glow on active controls for a living, film-score feel.
    - Large, readable parameter names with cinematic spacing.
*/
class NovemberResonanceAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit NovemberResonanceAudioProcessorEditor (NovemberResonanceAudioProcessor&);
    ~NovemberResonanceAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    NovemberResonanceAudioProcessor& processor;

    juce::Slider cutoffSlider, resonanceSlider, attackSlider, decaySlider, sustainSlider, releaseSlider, mixSlider, atSlider, vibSlider;
    juce::Label cutoffLabel, resonanceLabel, attackLabel, decayLabel, sustainLabel, releaseLabel, mixLabel, atLabel, vibLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> cutoffAttach, resAttach, attAttach, decAttach, susAttach, relAttach, mixAttach, atAttach, vibAttach;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NovemberResonanceAudioProcessorEditor)
};

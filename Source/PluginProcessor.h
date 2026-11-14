#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//==============================================================================
/** Today's Instrument: November Resonance
    A warm polyphonic pad synthesizer that breathes with channel aftertouch
    (modulating filter cutoff and subtle vibrato) and respects the sustain
    pedal for natural, live-feeling note holds. Dark cinematic atmospheres.
*/
class NovemberResonanceAudioProcessor : public juce::AudioProcessor
{
public:
    NovemberResonanceAudioProcessor();
    ~NovemberResonanceAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    juce::AudioProcessorValueTreeState apvts;

    // Simple polyphonic voice state (up to 8 voices)
    struct Voice
    {
        bool active = false;
        int note = -1;
        float velocity = 0.f;
        float phase1 = 0.f, phase2 = 0.f;
        float env = 0.f;
        float envTarget = 0.f;
        bool releasing = false;
        float filterState = 0.f;
    };

    static constexpr int maxVoices = 8;
    Voice voices[maxVoices];

    float aftertouch = 0.f;
    juce::SmoothedValue<float> smoothedAftertouch;
    juce::SmoothedValue<float> smoothedCutoff;
    juce::SmoothedValue<float> smoothedResonance;
    juce::SmoothedValue<float> smoothedMix;
    juce::SmoothedValue<float> smoothedVibrato;

    bool sustainPedalDown = false;
    juce::dsp::Limiter<float> limiter;

    void updateVoiceEnvelope (Voice& v, float attack, float decay, float sustain, float release, float sampleRate);
    int findFreeVoice();
    void noteOn (int note, float vel);
    void noteOff (int note);
    void processMIDI (const juce::MidiBuffer& midi);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NovemberResonanceAudioProcessor)
};

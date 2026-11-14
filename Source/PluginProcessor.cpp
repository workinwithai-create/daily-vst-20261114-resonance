#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NovemberResonanceAudioProcessor::NovemberResonanceAudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
}

NovemberResonanceAudioProcessor::~NovemberResonanceAudioProcessor() = default;

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout NovemberResonanceAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> ("cutoff", "Cutoff", 80.0f, 12000.0f, 1200.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("resonance", "Resonance", 0.1f, 0.95f, 0.4f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("attack", "Attack", 0.001f, 2.0f, 0.05f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("decay", "Decay", 0.001f, 2.0f, 0.3f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("sustain", "Sustain", 0.0f, 1.0f, 0.7f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("release", "Release", 0.001f, 5.0f, 0.8f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("mix", "Osc Mix", 0.0f, 1.0f, 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("atAmount", "Aftertouch Amount", 0.0f, 1.0f, 0.6f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> ("vibrato", "Vibrato Depth", 0.0f, 0.05f, 0.01f));

    return { params.begin(), params.end() };
}

//==============================================================================
void NovemberResonanceAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = 2;

    limiter.prepare (spec);
    limiter.setThreshold (-1.0f);
    limiter.setRelease (50.0f);

    smoothedAftertouch.reset (sampleRate, 0.05);
    smoothedCutoff.reset (sampleRate, 0.02);
    smoothedResonance.reset (sampleRate, 0.02);
    smoothedMix.reset (sampleRate, 0.02);
    smoothedVibrato.reset (sampleRate, 0.05);

    for (auto& v : voices)
        v = Voice{};
}

void NovemberResonanceAudioProcessor::releaseResources() {}

void NovemberResonanceAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    processMIDI (midi);

    auto cutoff = apvts.getRawParameterValue ("cutoff")->load();
    auto resonance = apvts.getRawParameterValue ("resonance")->load();
    auto attack = apvts.getRawParameterValue ("attack")->load();
    auto decay = apvts.getRawParameterValue ("decay")->load();
    auto sustain = apvts.getRawParameterValue ("sustain")->load();
    auto release = apvts.getRawParameterValue ("release")->load();
    auto mix = apvts.getRawParameterValue ("mix")->load();
    auto atAmount = apvts.getRawParameterValue ("atAmount")->load();
    auto vibratoDepth = apvts.getRawParameterValue ("vibrato")->load();

    smoothedCutoff.setTargetValue (cutoff);
    smoothedResonance.setTargetValue (resonance);
    smoothedMix.setTargetValue (mix);
    smoothedVibrato.setTargetValue (vibratoDepth);
    smoothedAftertouch.setTargetValue (aftertouch);

    const auto sampleRate = getSampleRate();
    const auto numSamples = buffer.getNumSamples();

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float at = smoothedAftertouch.getNextValue();
        float cut = smoothedCutoff.getNextValue() * (1.0f + at * atAmount * 2.0f);
        float res = smoothedResonance.getNextValue();
        float m = smoothedMix.getNextValue();
        float vib = smoothedVibrato.getNextValue() * at;

        float left = 0.f, right = 0.f;

        for (auto& v : voices)
        {
            if (! v.active) continue;

            updateVoiceEnvelope (v, attack, decay, sustain, release, sampleRate);

            if (v.env < 0.0001f && v.releasing)
            {
                v.active = false;
                continue;
            }

            float freq = juce::MidiMessage::getMidiNoteInHertz (v.note);
            float phaseInc1 = (freq * (1.0f + vib * std::sin (v.phase1 * 0.1f))) / sampleRate;
            float phaseInc2 = (freq * 1.01f) / sampleRate;

            v.phase1 += phaseInc1;
            v.phase2 += phaseInc2;
            if (v.phase1 >= 1.0f) v.phase1 -= 1.0f;
            if (v.phase2 >= 1.0f) v.phase2 -= 1.0f;

            // Simple saw + square
            float osc1 = 2.0f * v.phase1 - 1.0f; // saw
            float osc2 = (v.phase2 < 0.5f) ? 1.0f : -1.0f; // square
            float osc = osc1 * (1.0f - m) + osc2 * m;

            // Simple one-pole lowpass
            float alpha = std::exp (-2.0f * juce::MathConstants<float>::pi * cut / sampleRate);
            v.filterState = alpha * v.filterState + (1.0f - alpha) * osc * (1.0f + res * 2.0f);
            float filtered = v.filterState;

            float sampleVal = filtered * v.env * v.velocity * 0.3f;
            left += sampleVal;
            right += sampleVal;
        }

        buffer.setSample (0, sample, left);
        buffer.setSample (1, sample, right);
    }

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    limiter.process (context);
}

//==============================================================================
void NovemberResonanceAudioProcessor::processMIDI (const juce::MidiBuffer& midi)
{
    for (const auto metadata : midi)
    {
        auto msg = metadata.getMessage();
        if (msg.isNoteOn())
            noteOn (msg.getNoteNumber(), msg.getFloatVelocity());
        else if (msg.isNoteOff())
            noteOff (msg.getNoteNumber());
        else if (msg.isChannelPressure())
            aftertouch = msg.getChannelPressureValue() / 127.0f;
        else if (msg.isController() && msg.getControllerNumber() == 64)
        {
            bool wasDown = sustainPedalDown;
            sustainPedalDown = msg.getControllerValue() >= 64;
            if (wasDown && ! sustainPedalDown)
            {
                for (auto& v : voices)
                    if (v.active && ! v.releasing)
                        v.releasing = true;
            }
        }
    }
}

void NovemberResonanceAudioProcessor::noteOn (int note, float vel)
{
    int idx = findFreeVoice();
    if (idx < 0) return;

    auto& v = voices[idx];
    v.active = true;
    v.note = note;
    v.velocity = vel;
    v.phase1 = v.phase2 = 0.f;
    v.env = 0.f;
    v.releasing = false;
    v.filterState = 0.f;
}

void NovemberResonanceAudioProcessor::noteOff (int note)
{
    for (auto& v : voices)
    {
        if (v.active && v.note == note && ! v.releasing)
        {
            if (! sustainPedalDown)
                v.releasing = true;
        }
    }
}

int NovemberResonanceAudioProcessor::findFreeVoice()
{
    for (int i = 0; i < maxVoices; ++i)
        if (! voices[i].active)
            return i;
    return 0; // steal oldest
}

void NovemberResonanceAudioProcessor::updateVoiceEnvelope (Voice& v, float attack, float decay, float sustain, float release, float sampleRate)
{
    float attackRate = 1.0f / (attack * sampleRate + 1.0f);
    float decayRate = 1.0f / (decay * sampleRate + 1.0f);
    float releaseRate = 1.0f / (release * sampleRate + 1.0f);

    if (! v.releasing)
    {
        if (v.env < 1.0f)
            v.env += attackRate;
        else
            v.env = std::max (sustain, v.env - decayRate * (1.0f - sustain));
        if (v.env > 1.0f) v.env = 1.0f;
    }
    else
    {
        v.env -= releaseRate;
        if (v.env < 0.0f) v.env = 0.0f;
    }
}

//==============================================================================
void NovemberResonanceAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void NovemberResonanceAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr)
        if (xmlState->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessorEditor* NovemberResonanceAudioProcessor::createEditor()
{
    return new NovemberResonanceAudioProcessorEditor (*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NovemberResonanceAudioProcessor();
}

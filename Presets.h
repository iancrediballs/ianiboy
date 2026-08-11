#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include "Parameters.h"

// ============================================================================
//  Ianiboy factory presets - one per source type, per the blueprint:
//  varying degrees AND types of distortion fitted to the material.
//  Values are in PLAIN parameter units (dB where the param is dB, Hz, etc).
// ============================================================================

namespace ianiboy
{
    struct Preset
    {
        const char* name;
        std::vector<std::pair<const char*, float>> values;
    };

    // choice indices: midType { Tape=0, Tube, HardClip, Foldback, Diode, Sine, Bitcrush }
    //                 lowMode { Clean=0, Soft, Tape }   highType { TapeAir=0, CleanClip }
    inline const std::vector<Preset>& factoryPresets()
    {
        static const std::vector<Preset> presets = {

        { "Init (Transparent)", {
            { pid::inGain,0.f },{ pid::outGain,0.f },{ pid::dryMix,0.0f },
            { pid::lowCross,180.f },{ pid::highCross,3000.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,0 },{ pid::midDrive,1.f },{ pid::accentFreq,800.f },{ pid::accentDb,0.f },
            { pid::midGain,0.f },{ pid::tame,0.f },{ pid::tameFreq,4000.f },
            { pid::highType,0 },{ pid::highDrive,1.f },{ pid::air,0.f },{ pid::highGain,0.f },
            { pid::clipOn,0 },{ pid::clipCeil,-0.3f } } },

        // ---- VOCALS: parallel tube saturation, intelligibility intact --------
        { "Vocals - Parallel Heat", {
            { pid::lowCross,140.f },{ pid::highCross,3500.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,-1.f },
            { pid::midType,1 },{ pid::midDrive,4.5f },{ pid::accentFreq,1200.f },{ pid::accentDb,2.5f },
            { pid::midGain,-2.f },{ pid::tame,0.45f },{ pid::tameFreq,5200.f },
            { pid::highType,0 },{ pid::highDrive,1.4f },{ pid::air,4.f },{ pid::highGain,0.f },
            { pid::dryMix,0.35f },{ pid::clipOn,1 },{ pid::clipCeil,-0.5f } } },

        // ---- BASS: tube lows, controlled mid grit ---------------------------
        { "Bass - Anchor & Growl", {
            { pid::lowCross,180.f },{ pid::highCross,2500.f },
            { pid::lowMode,2 },{ pid::lowDrive,2.2f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,4 },{ pid::midDrive,6.f },{ pid::accentFreq,700.f },{ pid::accentDb,4.f },
            { pid::midGain,-1.5f },{ pid::tame,0.3f },{ pid::tameFreq,3500.f },
            { pid::highType,0 },{ pid::highDrive,1.2f },{ pid::air,1.f },{ pid::highGain,-2.f },
            { pid::dryMix,0.15f },{ pid::clipOn,1 },{ pid::clipCeil,-0.3f } } },

        // ---- KICK/RUMBLE: the Gemini serial-stage recipe --------------------
        { "Kick - Rumble Engine", {
            { pid::lowCross,90.f },{ pid::highCross,2200.f },
            { pid::lowMode,1 },{ pid::lowDrive,1.6f },{ pid::lowMono,1 },{ pid::lowGain,0.5f },
            { pid::midType,2 },{ pid::midDrive,9.f },{ pid::accentFreq,300.f },{ pid::accentDb,6.f },
            { pid::midGain,-2.f },{ pid::tame,0.5f },{ pid::tameFreq,2800.f },
            { pid::highType,1 },{ pid::highDrive,1.6f },{ pid::air,1.5f },{ pid::highGain,-1.f },
            { pid::dryMix,0.15f },{ pid::clipOn,1 },{ pid::clipCeil,-0.2f } } },

        // ---- DRUM BUS: hard-clip transients, glue ---------------------------
        { "Drums - Bus Crush", {
            { pid::lowCross,160.f },{ pid::highCross,3000.f },
            { pid::lowMode,2 },{ pid::lowDrive,1.5f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,2 },{ pid::midDrive,4.f },{ pid::accentFreq,900.f },{ pid::accentDb,2.f },
            { pid::midGain,-1.f },{ pid::tame,0.35f },{ pid::tameFreq,4500.f },
            { pid::highType,1 },{ pid::highDrive,1.5f },{ pid::air,3.f },{ pid::highGain,0.f },
            { pid::dryMix,0.25f },{ pid::clipOn,1 },{ pid::clipCeil,-0.2f } } },

        // ---- SYNTH LEAD: foldback screech ------------------------------------
        { "Synth - Screech Fold", {
            { pid::lowCross,200.f },{ pid::highCross,2800.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,-2.f },
            { pid::midType,3 },{ pid::midDrive,12.f },{ pid::accentFreq,1500.f },{ pid::accentDb,5.f },
            { pid::midGain,-3.f },{ pid::tame,0.55f },{ pid::tameFreq,5000.f },
            { pid::highType,1 },{ pid::highDrive,2.f },{ pid::air,4.f },{ pid::highGain,-1.f },
            { pid::dryMix,0.1f },{ pid::clipOn,1 },{ pid::clipCeil,-0.3f } } },

        // ---- SYNTH SIZZLE: bitcrush parallel sparkle -------------------------
        { "Synth - Digital Sizzle", {
            { pid::lowCross,200.f },{ pid::highCross,2500.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,6 },{ pid::midDrive,3.f },{ pid::accentFreq,2000.f },{ pid::accentDb,3.f },
            { pid::crushBits,8.f },{ pid::crushRate,6.f },
            { pid::midGain,-4.f },{ pid::tame,0.3f },{ pid::tameFreq,6000.f },
            { pid::highType,0 },{ pid::highDrive,1.8f },{ pid::air,5.f },{ pid::highGain,0.f },
            { pid::dryMix,0.4f },{ pid::clipOn,1 },{ pid::clipCeil,-0.4f } } },

        // ---- FX / TEXTURE: chaotic diode atmosphere --------------------------
        { "FX - Industrial Texture", {
            { pid::lowCross,250.f },{ pid::highCross,2200.f },
            { pid::lowMode,1 },{ pid::lowDrive,1.4f },{ pid::lowMono,1 },{ pid::lowGain,-1.f },
            { pid::midType,4 },{ pid::midDrive,14.f },{ pid::accentFreq,600.f },{ pid::accentDb,6.f },
            { pid::midGain,-4.f },{ pid::tame,0.4f },{ pid::tameFreq,3800.f },
            { pid::highType,1 },{ pid::highDrive,2.5f },{ pid::air,2.f },{ pid::highGain,-2.f },
            { pid::dryMix,0.05f },{ pid::clipOn,1 },{ pid::clipCeil,-0.3f } } },

        // ---- MASTER GLUE: gentle full-mix character --------------------------
        { "Master - Density Glue", {
            { pid::lowCross,120.f },{ pid::highCross,4000.f },
            { pid::lowMode,2 },{ pid::lowDrive,1.2f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,0 },{ pid::midDrive,1.8f },{ pid::accentFreq,800.f },{ pid::accentDb,1.f },
            { pid::midGain,0.f },{ pid::tame,0.2f },{ pid::tameFreq,4500.f },
            { pid::highType,0 },{ pid::highDrive,1.2f },{ pid::air,1.5f },{ pid::highGain,0.f },
            { pid::dryMix,0.2f },{ pid::clipOn,1 },{ pid::clipCeil,-1.0f } } },
        };
        return presets;
    }

    inline void applyPreset (juce::AudioProcessorValueTreeState& apvts, int index)
    {
        const auto& list = factoryPresets();
        if (index < 0 || index >= (int) list.size()) return;

        for (const auto& [id, value] : list[(size_t) index].values)
        {
            if (auto* param = apvts.getParameter (id))
            {
                param->beginChangeGesture();
                param->setValueNotifyingHost (param->convertTo0to1 (value));
                param->endChangeGesture();
            }
        }
    }
}

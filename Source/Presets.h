#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include "Parameters.h"

// ============================================================================
//  Ianiboy factory presets.
//  Each instrument has three escalating intensity tiers:
//     Medium      - tasteful, mix-ready character
//     Aggressive  - obvious, up-front saturation
//     Large       - extreme / sound-design territory
//  Values are in PLAIN parameter units (dB where dB, Hz, etc).
//  midType { Tape0, Tube1, HardClip2, Foldback3, Diode4, Sine5, Bitcrush6 }
//  lowMode { Clean0, Soft1, Tape2 }   highType { TapeAir0, CleanClip1 }
// ============================================================================

namespace ianiboy
{
    struct Preset
    {
        const char* name;
        std::vector<std::pair<const char*, float>> values;
    };

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

        // ===================== VOCALS (tube heat, parallel) =================
        { "Vocals - Medium", {
            { pid::lowCross,140.f },{ pid::highCross,3500.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,-1.f },
            { pid::midType,1 },{ pid::midDrive,3.f },{ pid::accentFreq,1200.f },{ pid::accentDb,2.f },
            { pid::midGain,-1.f },{ pid::tame,0.35f },{ pid::tameFreq,5200.f },
            { pid::highType,0 },{ pid::highDrive,1.3f },{ pid::air,3.f },{ pid::highGain,0.f },
            { pid::dryMix,0.35f },{ pid::clipOn,1 },{ pid::clipCeil,-0.8f } } },
        { "Vocals - Aggressive", {
            { pid::lowCross,140.f },{ pid::highCross,3500.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,-1.f },
            { pid::midType,1 },{ pid::midDrive,6.f },{ pid::accentFreq,1200.f },{ pid::accentDb,3.f },
            { pid::midGain,-2.5f },{ pid::tame,0.5f },{ pid::tameFreq,5000.f },
            { pid::highType,0 },{ pid::highDrive,1.5f },{ pid::air,4.f },{ pid::highGain,0.f },
            { pid::dryMix,0.25f },{ pid::clipOn,1 },{ pid::clipCeil,-0.5f } } },
        { "Vocals - Large", {
            { pid::lowCross,150.f },{ pid::highCross,3200.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,-2.f },
            { pid::midType,4 },{ pid::midDrive,11.f },{ pid::accentFreq,1400.f },{ pid::accentDb,5.f },
            { pid::midGain,-4.f },{ pid::tame,0.6f },{ pid::tameFreq,4800.f },
            { pid::highType,1 },{ pid::highDrive,1.8f },{ pid::air,5.f },{ pid::highGain,-1.f },
            { pid::dryMix,0.15f },{ pid::clipOn,1 },{ pid::clipCeil,-0.4f } } },

        // ===================== BASS =========================================
        { "Bass - Medium", {
            { pid::lowCross,180.f },{ pid::highCross,2500.f },
            { pid::lowMode,2 },{ pid::lowDrive,1.6f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,1 },{ pid::midDrive,3.f },{ pid::accentFreq,700.f },{ pid::accentDb,3.f },
            { pid::midGain,-1.f },{ pid::tame,0.25f },{ pid::tameFreq,3500.f },
            { pid::highType,0 },{ pid::highDrive,1.2f },{ pid::air,1.f },{ pid::highGain,-1.f },
            { pid::dryMix,0.2f },{ pid::clipOn,1 },{ pid::clipCeil,-0.4f } } },
        { "Bass - Aggressive", {
            { pid::lowCross,180.f },{ pid::highCross,2500.f },
            { pid::lowMode,2 },{ pid::lowDrive,2.4f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,4 },{ pid::midDrive,6.f },{ pid::accentFreq,700.f },{ pid::accentDb,4.f },
            { pid::midGain,-2.f },{ pid::tame,0.35f },{ pid::tameFreq,3500.f },
            { pid::highType,0 },{ pid::highDrive,1.4f },{ pid::air,1.5f },{ pid::highGain,-2.f },
            { pid::dryMix,0.12f },{ pid::clipOn,1 },{ pid::clipCeil,-0.3f } } },
        { "Bass - Large", {
            { pid::lowCross,190.f },{ pid::highCross,2400.f },
            { pid::lowMode,2 },{ pid::lowDrive,3.f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,3 },{ pid::midDrive,11.f },{ pid::accentFreq,750.f },{ pid::accentDb,6.f },
            { pid::midGain,-4.f },{ pid::tame,0.45f },{ pid::tameFreq,3200.f },
            { pid::highType,1 },{ pid::highDrive,1.8f },{ pid::air,2.f },{ pid::highGain,-2.f },
            { pid::dryMix,0.08f },{ pid::clipOn,1 },{ pid::clipCeil,-0.3f } } },

        // ===================== DRUMS (bus) =================================
        { "Drums - Medium", {
            { pid::lowCross,160.f },{ pid::highCross,3000.f },
            { pid::lowMode,2 },{ pid::lowDrive,1.4f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,2 },{ pid::midDrive,3.f },{ pid::accentFreq,900.f },{ pid::accentDb,2.f },
            { pid::midGain,-1.f },{ pid::tame,0.3f },{ pid::tameFreq,4500.f },
            { pid::highType,1 },{ pid::highDrive,1.4f },{ pid::air,3.f },{ pid::highGain,0.f },
            { pid::dryMix,0.3f },{ pid::clipOn,1 },{ pid::clipCeil,-0.3f } } },
        { "Drums - Aggressive", {
            { pid::lowCross,160.f },{ pid::highCross,3000.f },
            { pid::lowMode,2 },{ pid::lowDrive,1.6f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,2 },{ pid::midDrive,5.f },{ pid::accentFreq,900.f },{ pid::accentDb,3.f },
            { pid::midGain,-1.5f },{ pid::tame,0.4f },{ pid::tameFreq,4500.f },
            { pid::highType,1 },{ pid::highDrive,1.6f },{ pid::air,3.5f },{ pid::highGain,0.f },
            { pid::dryMix,0.2f },{ pid::clipOn,1 },{ pid::clipCeil,-0.2f } } },
        { "Drums - Large", {
            { pid::lowCross,170.f },{ pid::highCross,3000.f },
            { pid::lowMode,2 },{ pid::lowDrive,1.8f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,2 },{ pid::midDrive,8.f },{ pid::accentFreq,1000.f },{ pid::accentDb,4.f },
            { pid::midGain,-2.f },{ pid::tame,0.5f },{ pid::tameFreq,4200.f },
            { pid::highType,1 },{ pid::highDrive,2.f },{ pid::air,4.f },{ pid::highGain,-1.f },
            { pid::dryMix,0.12f },{ pid::clipOn,1 },{ pid::clipCeil,-0.2f } } },

        // ===================== KICK (serial rumble) =======================
        { "Kick - Medium", {
            { pid::lowCross,90.f },{ pid::highCross,2200.f },
            { pid::lowMode,1 },{ pid::lowDrive,1.4f },{ pid::lowMono,1 },{ pid::lowGain,0.5f },
            { pid::midType,2 },{ pid::midDrive,5.f },{ pid::accentFreq,300.f },{ pid::accentDb,5.f },
            { pid::midGain,-1.5f },{ pid::tame,0.4f },{ pid::tameFreq,2800.f },
            { pid::highType,1 },{ pid::highDrive,1.4f },{ pid::air,1.5f },{ pid::highGain,-1.f },
            { pid::dryMix,0.2f },{ pid::clipOn,1 },{ pid::clipCeil,-0.3f } } },
        { "Kick - Aggressive", {
            { pid::lowCross,90.f },{ pid::highCross,2200.f },
            { pid::lowMode,1 },{ pid::lowDrive,1.6f },{ pid::lowMono,1 },{ pid::lowGain,0.5f },
            { pid::midType,2 },{ pid::midDrive,8.f },{ pid::accentFreq,300.f },{ pid::accentDb,6.f },
            { pid::midGain,-2.f },{ pid::tame,0.5f },{ pid::tameFreq,2800.f },
            { pid::highType,1 },{ pid::highDrive,1.6f },{ pid::air,1.5f },{ pid::highGain,-1.f },
            { pid::dryMix,0.15f },{ pid::clipOn,1 },{ pid::clipCeil,-0.2f } } },
        { "Kick - Large", {
            { pid::lowCross,95.f },{ pid::highCross,2200.f },
            { pid::lowMode,1 },{ pid::lowDrive,1.8f },{ pid::lowMono,1 },{ pid::lowGain,1.f },
            { pid::midType,4 },{ pid::midDrive,13.f },{ pid::accentFreq,320.f },{ pid::accentDb,7.f },
            { pid::midGain,-3.f },{ pid::tame,0.55f },{ pid::tameFreq,2600.f },
            { pid::highType,1 },{ pid::highDrive,1.8f },{ pid::air,2.f },{ pid::highGain,-1.f },
            { pid::dryMix,0.1f },{ pid::clipOn,1 },{ pid::clipCeil,-0.2f } } },

        // ===================== SYNTH (lead / stab) ========================
        { "Synth - Medium", {
            { pid::lowCross,200.f },{ pid::highCross,2800.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,-1.f },
            { pid::midType,3 },{ pid::midDrive,6.f },{ pid::accentFreq,1500.f },{ pid::accentDb,4.f },
            { pid::midGain,-2.f },{ pid::tame,0.45f },{ pid::tameFreq,5000.f },
            { pid::highType,0 },{ pid::highDrive,1.5f },{ pid::air,3.f },{ pid::highGain,-1.f },
            { pid::dryMix,0.15f },{ pid::clipOn,1 },{ pid::clipCeil,-0.4f } } },
        { "Synth - Aggressive", {
            { pid::lowCross,200.f },{ pid::highCross,2800.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,-2.f },
            { pid::midType,3 },{ pid::midDrive,10.f },{ pid::accentFreq,1500.f },{ pid::accentDb,5.f },
            { pid::midGain,-3.f },{ pid::tame,0.55f },{ pid::tameFreq,5000.f },
            { pid::highType,1 },{ pid::highDrive,2.f },{ pid::air,4.f },{ pid::highGain,-1.f },
            { pid::dryMix,0.1f },{ pid::clipOn,1 },{ pid::clipCeil,-0.3f } } },
        { "Synth - Large", {
            { pid::lowCross,220.f },{ pid::highCross,2600.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,-3.f },
            { pid::midType,3 },{ pid::midDrive,16.f },{ pid::accentFreq,1700.f },{ pid::accentDb,6.f },
            { pid::midGain,-4.f },{ pid::tame,0.65f },{ pid::tameFreq,4800.f },
            { pid::highType,1 },{ pid::highDrive,2.5f },{ pid::air,5.f },{ pid::highGain,-2.f },
            { pid::dryMix,0.06f },{ pid::clipOn,1 },{ pid::clipCeil,-0.3f } } },

        // ===================== MASTER / BUS GLUE ==========================
        { "Master - Medium", {
            { pid::lowCross,120.f },{ pid::highCross,4000.f },
            { pid::lowMode,2 },{ pid::lowDrive,1.2f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,0 },{ pid::midDrive,1.6f },{ pid::accentFreq,800.f },{ pid::accentDb,1.f },
            { pid::midGain,0.f },{ pid::tame,0.2f },{ pid::tameFreq,4500.f },
            { pid::highType,0 },{ pid::highDrive,1.2f },{ pid::air,1.5f },{ pid::highGain,0.f },
            { pid::dryMix,0.2f },{ pid::clipOn,1 },{ pid::clipCeil,-1.0f } } },
        { "Master - Aggressive", {
            { pid::lowCross,120.f },{ pid::highCross,4000.f },
            { pid::lowMode,2 },{ pid::lowDrive,1.4f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,0 },{ pid::midDrive,2.4f },{ pid::accentFreq,800.f },{ pid::accentDb,1.5f },
            { pid::midGain,0.f },{ pid::tame,0.3f },{ pid::tameFreq,4500.f },
            { pid::highType,0 },{ pid::highDrive,1.3f },{ pid::air,2.f },{ pid::highGain,0.f },
            { pid::dryMix,0.15f },{ pid::clipOn,1 },{ pid::clipCeil,-0.7f } } },
        { "Master - Large", {
            { pid::lowCross,120.f },{ pid::highCross,4200.f },
            { pid::lowMode,2 },{ pid::lowDrive,1.6f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,0 },{ pid::midDrive,3.2f },{ pid::accentFreq,850.f },{ pid::accentDb,2.f },
            { pid::midGain,0.f },{ pid::tame,0.35f },{ pid::tameFreq,4500.f },
            { pid::highType,0 },{ pid::highDrive,1.4f },{ pid::air,2.5f },{ pid::highGain,0.f },
            { pid::dryMix,0.1f },{ pid::clipOn,1 },{ pid::clipCeil,-0.5f } } },

        // ===================== SPECIALS ===================================
        { "FX - Industrial Texture", {
            { pid::lowCross,250.f },{ pid::highCross,2200.f },
            { pid::lowMode,1 },{ pid::lowDrive,1.4f },{ pid::lowMono,1 },{ pid::lowGain,-1.f },
            { pid::midType,4 },{ pid::midDrive,14.f },{ pid::accentFreq,600.f },{ pid::accentDb,6.f },
            { pid::midGain,-4.f },{ pid::tame,0.4f },{ pid::tameFreq,3800.f },
            { pid::highType,1 },{ pid::highDrive,2.5f },{ pid::air,2.f },{ pid::highGain,-2.f },
            { pid::dryMix,0.05f },{ pid::clipOn,1 },{ pid::clipCeil,-0.3f } } },
        { "Synth - Digital Sizzle", {
            { pid::lowCross,200.f },{ pid::highCross,2500.f },
            { pid::lowMode,0 },{ pid::lowDrive,1.f },{ pid::lowMono,1 },{ pid::lowGain,0.f },
            { pid::midType,6 },{ pid::midDrive,3.f },{ pid::accentFreq,2000.f },{ pid::accentDb,3.f },
            { pid::crushBits,8.f },{ pid::crushRate,6.f },
            { pid::midGain,-4.f },{ pid::tame,0.3f },{ pid::tameFreq,6000.f },
            { pid::highType,0 },{ pid::highDrive,1.8f },{ pid::air,5.f },{ pid::highGain,0.f },
            { pid::dryMix,0.4f },{ pid::clipOn,1 },{ pid::clipCeil,-0.4f } } },
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

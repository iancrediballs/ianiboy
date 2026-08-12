cmake_minimum_required(VERSION 3.22)

# ============================================================================
#  IANIBOY - Sonic Architecture Character Processor
#  Multiband, oversampled saturation engine built on JUCE.
#  Targets: VST3 (Win/mac/Linux) + AU (mac)
# ============================================================================

project(Ianiboy VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Pull JUCE (pinned, reproducible) --------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# --- Formats ---------------------------------------------------------------
# AU only builds on macOS; JUCE ignores it elsewhere but we gate it anyway.
if(APPLE)
    set(IANIBOY_FORMATS AU VST3 Standalone)
else()
    set(IANIBOY_FORMATS VST3 Standalone)
endif()

juce_add_plugin(Ianiboy
    COMPANY_NAME                "Ianiboy Audio"
    BUNDLE_ID                   "com.ianiboy.ianiboy"
    PLUGIN_MANUFACTURER_CODE    Ianb
    PLUGIN_CODE                 Iby1
    FORMATS                     ${IANIBOY_FORMATS}
    PRODUCT_NAME                "Ianiboy"
    IS_SYNTH                    FALSE
    NEEDS_MIDI_INPUT            FALSE
    NEEDS_MIDI_OUTPUT           FALSE
    COPY_PLUGIN_AFTER_BUILD     TRUE
    VST3_CATEGORIES             "Fx" "Distortion"
    AU_MAIN_TYPE                "kAudioUnitType_Effect"
)

target_sources(Ianiboy PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)

target_compile_definitions(Ianiboy PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(Ianiboy
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

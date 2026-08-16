#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Presets.h"

// ============================================================================
//  PresetManager - factory presets (compiled in) + user presets (XML files
//  in the user's app-data folder). Powers the preset browser.
// ============================================================================

namespace ianiboy
{
    class PresetManager
    {
    public:
        explicit PresetManager (juce::AudioProcessorValueTreeState& s) : apvts (s)
        {
            userDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                        .getChildFile ("Ianiboy").getChildFile ("Presets");
            userDir.createDirectory();
            refresh();
        }

        void refresh()
        {
            userFiles.clear();
            userNames.clear();
            for (auto& f : userDir.findChildFiles (juce::File::findFiles, false, "*.xml"))
            {
                userFiles.add (f);
                userNames.add (f.getFileNameWithoutExtension());
            }
        }

        int  numFactory() const              { return (int) factoryPresets().size(); }
        int  numPresets() const              { return numFactory() + userFiles.size(); }
        bool isUser (int index) const        { return index >= numFactory(); }

        juce::StringArray allNames() const
        {
            juce::StringArray names;
            for (auto& p : factoryPresets()) names.add (p.name);
            for (auto& n : userNames)        names.add (n);
            return names;
        }

        void loadPreset (int index)
        {
            if (index < 0 || index >= numPresets()) return;

            if (index < numFactory())
            {
                applyPreset (apvts, index);
            }
            else
            {
                const auto file = userFiles[index - numFactory()];
                if (auto xml = juce::XmlDocument::parse (file))
                    if (xml->hasTagName (apvts.state.getType()))
                        apvts.replaceState (juce::ValueTree::fromXml (*xml));
            }
        }

        // Returns the index of the saved preset (in the combined list), or -1.
        int saveUserPreset (const juce::String& rawName)
        {
            auto name = rawName.trim();
            if (name.isEmpty()) return -1;
            name = juce::File::createLegalFileName (name);

            auto file = userDir.getChildFile (name + ".xml");
            if (auto xml = apvts.copyState().createXml())
                xml->writeTo (file);

            refresh();
            const int userIdx = userNames.indexOf (name);
            return userIdx >= 0 ? numFactory() + userIdx : -1;
        }

        bool deleteUserPreset (int index)
        {
            if (! isUser (index)) return false;
            const bool ok = userFiles[index - numFactory()].deleteFile();
            refresh();
            return ok;
        }

    private:
        juce::AudioProcessorValueTreeState& apvts;
        juce::File userDir;
        juce::Array<juce::File> userFiles;
        juce::StringArray userNames;
    };
}

/*
  ==============================================================================

    MTCGenEditor.h
    Created: 15 Apr 2025
    Author:  Sebastian

  ==============================================================================
*/

//==============================================================================
//  MTCGenEditor.h
//==============================================================================

#ifndef MTCGENEDITOR_H_INCLUDED
#define MTCGENEDITOR_H_INCLUDED

#include <JuceHeader.h>
#include "MTCGenProcessor.h"
#include "MappingTableComponent.h"
#include "MidiOutputSelector.h"

/**
 * @brief The main GUI editor for the MTCGen plugin.
 */
class MTCGenAudioProcessorEditor : public juce::AudioProcessorEditor,
    public juce::Timer,
    public juce::ComboBox::Listener
{
public:
    MTCGenAudioProcessorEditor(MTCGenAudioProcessor& p);
    ~MTCGenAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void comboBoxChanged(juce::ComboBox*) override;

private:
    MTCGenAudioProcessor& processor;
    MappingTableComponent   mappingTable;
    MidiOutputSelector      midiOutputSelector;

    juce::Label             currentTimecodeLabel;
    juce::ComboBox          frameRateComboBox;
    juce::ComboBox          mtcFormatComboBox;

    // Inline debug panel
    juce::ToggleButton      debugToggle{ "Debug" };
    juce::TextEditor        debugPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MTCGenAudioProcessorEditor)
};

#endif // MTCGENEDITOR_H_INCLUDED

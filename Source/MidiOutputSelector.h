/*
  ==============================================================================

    MidiOutputSelector.h
    Created: 15 Apr 2025
    Author:  Sebastian

  ==============================================================================
*/

#ifndef MIDIOUTPUTSELECTOR_H_INCLUDED
#define MIDIOUTPUTSELECTOR_H_INCLUDED

#include <JuceHeader.h>
#include "MTCGenProcessor.h"

/**
 * @brief A UI component that displays available MIDI outputs with checkboxes.
 *
 * Users can select one or more outputs. Selections are reflected by ToggleButtons.
 * When the selection changes, the processor is updated.
 */
class MidiOutputSelector : public juce::Component,
                           public juce::ListBoxModel
{
public:
    /**
     * @brief Constructs the MIDI output selector.
     * @param proc Reference to the MTCGenAudioProcessor.
     */
    MidiOutputSelector(MTCGenAudioProcessor& proc);
    
    ~MidiOutputSelector() override;
    
    // ListBoxModel methods.
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g,
                           int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent& event) override;
    juce::Component* refreshComponentForRow (int rowNumber, bool isRowSelected, juce::Component* existingComponent) override;
    
    // Component override.
    void resized() override;
    
private:
    /** Updates the processor with the indices of selected outputs. */
    void updateProcessorOutputs();
    
    MTCGenAudioProcessor& processor;
    juce::ListBox listBox { "MidiOutputList" };
    juce::Array<bool> selectedStates;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiOutputSelector)
};

#endif // MIDIOUTPUTSELECTOR_H_INCLUDED

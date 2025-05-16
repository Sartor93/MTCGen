/*
  ==============================================================================

    MappingTableComponent.h
    Created: 15 Apr 2025
    Author:  Sebastian

  ==============================================================================
*/

#ifndef MAPPINGTABLECOMPONENT_H_INCLUDED
#define MAPPINGTABLECOMPONENT_H_INCLUDED

#include <JuceHeader.h>
#include "MTCGenProcessor.h"

/**
 * @brief A component for displaying and editing mapping entries.
 *
 * Columns:
 * 1. Label           (editable)
 * 2. MIDI Note       (editable)
 * 3. Mapping Timecode(editable)
 * 4. Start           (read‑only)
 * 5. Set Start       (button)
 * 6. Delete          (button)
 */
class MappingTableComponent : public juce::Component,
    public juce::TableListBoxModel,
    public juce::Button::Listener
{
public:
    MappingTableComponent(MTCGenAudioProcessor& proc);
    ~MappingTableComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    int getNumRows() override;

    void paintRowBackground(juce::Graphics&, int row, int width, int height, bool) override;
    void paintCell(juce::Graphics&, int row, int columnId, int width, int height, bool) override;
    juce::Component* refreshComponentForCell(int row, int columnId, bool, juce::Component*) override;

    void buttonClicked(juce::Button* button) override;
    void refreshTable() { table.updateContent(); }

private:
    MTCGenAudioProcessor& processor;
    juce::TableListBox    table;
    juce::TextButton      addMappingButton{ "Add Mapping" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MappingTableComponent)
};

#endif // MAPPINGTABLECOMPONENT_H_INCLUDED

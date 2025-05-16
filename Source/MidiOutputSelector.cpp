/*
  ==============================================================================

    MidiOutputSelector.cpp
    Created: 15 Apr 2025
    Author:  Sebastian

  ==============================================================================
*/

#include "MidiOutputSelector.h"

MidiOutputSelector::MidiOutputSelector(MTCGenAudioProcessor& proc)
    : processor(proc)
{
    listBox.setModel(this);
    addAndMakeVisible(listBox);
    
    auto names = processor.getAvailableMidiOutputNames();
    for (int i = 0; i < names.size(); ++i)
        selectedStates.add(false);
}

MidiOutputSelector::~MidiOutputSelector()
{
}

int MidiOutputSelector::getNumRows()
{
    return processor.getAvailableMidiOutputNames().size();
}

void MidiOutputSelector::paintListBoxItem (int rowNumber, juce::Graphics& g,
                                           int width, int height, bool /*rowIsSelected*/)
{
    // This method is unused when using refreshComponentForRow.
}

juce::Component* MidiOutputSelector::refreshComponentForRow (int rowNumber, bool, juce::Component* existingComponent)
{
    if (rowNumber < 0 || rowNumber >= selectedStates.size())
        return nullptr;
    
    juce::ToggleButton* toggle = dynamic_cast<juce::ToggleButton*>(existingComponent);
    if (toggle == nullptr)
    {
        toggle = new juce::ToggleButton();
        toggle->onClick = [this, rowNumber, toggle]()
        {
            selectedStates.set(rowNumber, toggle->getToggleState());
            updateProcessorOutputs();
        };
    }
    auto names = processor.getAvailableMidiOutputNames();
    if (rowNumber < names.size())
        toggle->setButtonText(names[rowNumber]);
    toggle->setToggleState(selectedStates[rowNumber], juce::dontSendNotification);
    return toggle;
}

void MidiOutputSelector::listBoxItemClicked (int row, const juce::MouseEvent& event)
{
    // Clicking on the row is handled by the toggle button.
}

void MidiOutputSelector::resized()
{
    listBox.setBounds(getLocalBounds());
}

void MidiOutputSelector::updateProcessorOutputs()
{
    juce::Array<int> indices;
    for (int i = 0; i < selectedStates.size(); ++i)
    {
        if (selectedStates[i])
            indices.add(i);
    }
    processor.setSelectedMidiOutputs(indices);
}

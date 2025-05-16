/**
 * @file MappingTableComponent.cpp
 * @brief UI table for editing MIDI-Timecode mappings.
 */

#include "MappingTableComponent.h"

//==============================================================================
/**
 * @brief Construct the MappingTableComponent, set up the table and header.
 */
MappingTableComponent::MappingTableComponent(MTCGenAudioProcessor& proc)
    : processor(proc)
{
    addAndMakeVisible(addMappingButton);
    addMappingButton.setButtonText("Add Mapping");
    addMappingButton.addListener(this);

    addAndMakeVisible(table);
    table.setModel(this);

    auto& h = table.getHeader();
    h.addColumn("Label", 1, 120);
    h.addColumn("MIDI Note", 2, 100);
    h.addColumn("Mapping TC", 3, 150);
    h.addColumn("Start", 4, 150);
    h.addColumn("", 5, 80);  // Set Start
    h.addColumn("End", 6, 150);
    h.addColumn("", 7, 80);  // Set End
    h.addColumn("", 8, 80);  // Delete
    h.setStretchToFitActive(true);
}

/** Destructor */
MappingTableComponent::~MappingTableComponent() {}

//==============================================================================
int MappingTableComponent::getNumRows()
{
    return static_cast<int>(processor.getMappings().size());
}

void MappingTableComponent::paintRowBackground(juce::Graphics& g,
    int row, int, int, bool)
{
    if (row == processor.getActiveMappingIndex())
        g.fillAll(juce::Colours::lightblue);
    else
        g.fillAll(juce::Colours::white);
}

/**
 * @brief Paint the text for the Start and End columns.
 */
 // paintCell: only draw columns 1–3 here; Start/End (4,6) are handled by components
void MappingTableComponent::paintCell(juce::Graphics& g,
    int row, int columnId,
    int width, int height,
    bool /*selected*/)
{
    if (row < 0 || row >= getNumRows())
        return;

    const auto& m = processor.getMappings()[row];
    juce::String text;

    switch (columnId)
    {
    case 1: // Label
        text = m.getLabel();
        break;

    case 2: // MIDI Note
        text = juce::MidiMessage::getMidiNoteName(
            m.getMidiNote(), true, true, 4);
        break;

    case 3: // Mapping Timecode
        text = m.getTimecodeString();
        break;

    default:
        return; // columns 4,5,6,7,8 use components
    }

    g.setColour(juce::Colours::black);
    g.drawText(text,
        2, 0,
        width - 4, height,
        juce::Justification::centredLeft,
        true);
}

/**
 * @brief Provide in-cell editors and buttons for each column.
 */
juce::Component* MappingTableComponent::refreshComponentForCell(int row,
    int columnId,
    bool /*selected*/,
    juce::Component* existing)
{
    if (row < 0 || row >= getNumRows())
        return nullptr;

    auto& m = processor.getMappings()[row];

    // 1) Label editor
    if (columnId == 1)
    {
        auto* ed = dynamic_cast<juce::TextEditor*>(existing);
        if (!ed)
        {
            ed = new juce::TextEditor();
            ed->setFont(14.0f);
            ed->onTextChange = [this, row, ed]()
                {
                    processor.getMappings()[row].setLabel(ed->getText());
                };
        }
        ed->setText(m.getLabel(), juce::dontSendNotification);
        return ed;
    }

    // 2) MIDI Note ComboBox
    if (columnId == 2)
    {
        auto* cb = dynamic_cast<juce::ComboBox*>(existing);
        if (!cb)
        {
            cb = new juce::ComboBox();
            static const char* names[] = { "C","C#","D","D#","E","F","F#","G","G#","A","A#","B" };
            for (int i = 0; i < 128; ++i)
                cb->addItem(names[i % 12] + juce::String(i / 12 - 1), i + 1);
            cb->onChange = [this, row, cb]()
                {
                    processor.getMappings()[row].setMidiNote(cb->getSelectedId() - 1);
                };
        }
        cb->setSelectedId(m.getMidiNote() + 1, juce::dontSendNotification);
        return cb;
    }

    // 3) Mapping Timecode editor
    if (columnId == 3)
    {
        auto* ed = dynamic_cast<juce::TextEditor*>(existing);
        if (!ed)
        {
            ed = new juce::TextEditor();
            ed->setFont(14.0f);
            ed->onTextChange = [this, row, ed]()
                {
                    processor.getMappings()[row].setTimecodeString(ed->getText());
                };
        }
        ed->setText(m.getTimecodeString(), juce::dontSendNotification);
        return ed;
    }

    // 4 & 6) Start and End as read-only TextEditors
    if (columnId == 4 || columnId == 6)
    {
        auto* ed = dynamic_cast<juce::TextEditor*>(existing);
        if (!ed)
        {
            ed = new juce::TextEditor();
            ed->setFont(14.0f);
            ed->setReadOnly(true);
            ed->setJustification(juce::Justification::centredLeft);
        }

        double t = (columnId == 4)
            ? m.getDetectedStartTime()
            : m.getDetectedEndTime();

        juce::String txt;
        if (t >= 0.0)
        {
            int tot = int(t);
            int hh = tot / 3600;
            int mm = (tot % 3600) / 60;
            int ss = tot % 60;
            int ff = int((t - tot) * processor.getFrameRate());
            txt = juce::String::formatted("%02d:%02d:%02d:%02d", hh, mm, ss, ff);
        }

        ed->setText(txt, juce::dontSendNotification);
        return ed;
    }

    // 5) Set Start button
    if (columnId == 5)
    {
        auto* btn = dynamic_cast<juce::TextButton*>(existing);
        if (!btn)
        {
            btn = new juce::TextButton("Set Start");
            btn->onClick = [this, row]()
                {
                    double now = processor.getPlayheadTime();
                    auto& mm = processor.getMappings()[row];
                    mm.setDetectedStartTime(now);
                    mm.setDetectedEndTime(-1.0);
                    mm.setIsActive(false);
                    table.updateContent();
                };
        }
        return btn;
    }

    // 7) Set End button
    if (columnId == 7)
    {
        auto* btn = dynamic_cast<juce::TextButton*>(existing);
        if (!btn)
        {
            btn = new juce::TextButton("Set End");
            btn->onClick = [this, row]()
                {
                    double now = processor.getPlayheadTime();
                    auto& mm = processor.getMappings()[row];
                    mm.setDetectedEndTime(now);
                    mm.setIsActive(false);
                    table.updateContent();
                };
        }
        return btn;
    }

    // 8) Delete button
    if (columnId == 8)
    {
        auto* btn = dynamic_cast<juce::TextButton*>(existing);
        if (!btn)
        {
            btn = new juce::TextButton("Delete");
            btn->onClick = [this, row]()
                {
                    processor.removeMapping(row);
                    table.updateContent();
                };
        }
        return btn;
    }

    return nullptr;
}

/**
 * @brief Called when the "Add Mapping" button is clicked.
 */
void MappingTableComponent::buttonClicked(juce::Button* b)
{
    if (b == &addMappingButton)
    {
        auto& v = processor.getMappings();
        int note = v.empty() ? 60 : v.back().getMidiNote() + 1;
        v.emplace_back("00:00:00:00", note, "New Mapping");
        table.updateContent();
    }
}

//==============================================================================
// Paint the component background
void MappingTableComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::lightgrey);
}

// Layout the button and the table
void MappingTableComponent::resized()
{
    auto area = getLocalBounds().reduced(4);
    addMappingButton.setBounds(area.removeFromTop(30));
    table.setBounds(area);
}

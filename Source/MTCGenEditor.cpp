/**
 * @file MTCGenEditor.cpp
 * @brief UI editor for MTCGenAudioProcessor.
 */

#include "MTCGenEditor.h"

 //==============================================================================
 /**
  * @brief Ctor: builds controls, mapping table, and debug panel.
  * @param p Reference to the processor.
  */
MTCGenAudioProcessorEditor::MTCGenAudioProcessorEditor(MTCGenAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p), mappingTable(processor),
    midiOutputSelector(processor)
{
    setSize(600, 700);

    addAndMakeVisible(mappingTable);
    addAndMakeVisible(midiOutputSelector);

    currentTimecodeLabel.setJustificationType(juce::Justification::centred);
    currentTimecodeLabel.setFont(juce::Font("Consolas", 36.0f, juce::Font::plain));
    currentTimecodeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::black);
    currentTimecodeLabel.setColour(juce::Label::textColourId, juce::Colours::lime);
    currentTimecodeLabel.setBorderSize(juce::BorderSize<int>(2));
    currentTimecodeLabel.setText("Timecode: --:--:--:--", juce::dontSendNotification);
    addAndMakeVisible(currentTimecodeLabel);

    frameRateComboBox.addItem("24", 1);
    frameRateComboBox.addItem("25", 2);
    frameRateComboBox.addItem("29.97", 3);
    frameRateComboBox.addItem("30", 4);
    frameRateComboBox.setSelectedId(
        processor.getFrameRate() == 24.0 ? 1 :
        processor.getFrameRate() == 25.0 ? 2 :
        processor.getFrameRate() == 29.97 ? 3 : 4,
        juce::dontSendNotification);
    frameRateComboBox.addListener(this);
    addAndMakeVisible(frameRateComboBox);

    mtcFormatComboBox.addItem("Full SysEx", 1);
    mtcFormatComboBox.addItem("Quarter Frame", 2);
    mtcFormatComboBox.setSelectedId(
        processor.getMTCFormat() == FullSysEx ? 1 : 2,
        juce::dontSendNotification);
    mtcFormatComboBox.addListener(this);
    addAndMakeVisible(mtcFormatComboBox);

    debugToggle.setButtonText("Show Debug");
    debugToggle.onClick = [this]() {
        debugPanel.setVisible(debugToggle.getToggleState());
        resized();
        };
    addAndMakeVisible(debugToggle);

    debugPanel.setMultiLine(true);
    debugPanel.setReadOnly(true);
    debugPanel.setFont(juce::Font("Consolas", 12.0f, juce::Font::plain));
    debugPanel.setColour(juce::TextEditor::backgroundColourId, juce::Colours::black);
    debugPanel.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    debugPanel.setVisible(false);
    addAndMakeVisible(debugPanel);

    startTimer(100); // 10 Hz UI update
}

/** Destructor */
MTCGenAudioProcessorEditor::~MTCGenAudioProcessorEditor() {}

/** @brief Paints the editor background. */
void MTCGenAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

/** @brief Lays out all child components. */
void MTCGenAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    auto topArea = area.removeFromTop(area.getHeight() * 0.45f);
    mappingTable.setBounds(topArea);

    auto midiArea = area.removeFromTop(150);
    midiOutputSelector.setBounds(midiArea);

    auto ctrl = area.removeFromTop(40);
    frameRateComboBox.setBounds(ctrl.removeFromLeft(150));
    mtcFormatComboBox.setBounds(ctrl.removeFromLeft(150).reduced(5));

    currentTimecodeLabel.setBounds(area.removeFromTop(80));

    auto dbgToggleArea = area.removeFromTop(24).removeFromLeft(100).reduced(4);
    debugToggle.setBounds(dbgToggleArea);

    if (debugPanel.isVisible())
        debugPanel.setBounds(area.reduced(4));
}

/**
 * @brief Timer callback: updates timecode display, table, and debug log.
 */
void MTCGenAudioProcessorEditor::timerCallback()
{
    processor.updateTimecodeFromPlayHead();

    auto tc = processor.getCurrentTimecode();
    currentTimecodeLabel.setText(
        tc.isEmpty() ? "Timecode: --:--:--:--"
        : "Timecode: " + tc,
        juce::dontSendNotification);

    mappingTable.refreshTable();

    if (debugPanel.isVisible())
    {
        auto events = processor.getDebugEvents();
        juce::String log;
        for (auto& e : events)
            log += juce::String(e.time, 3) + " : " + e.desc + "\n";
        debugPanel.setText(log, juce::dontSendNotification);
    }
}

/**
 * @brief Handles changes to frame-rate or MTC-format ComboBoxes.
 * @param cb Pointer to the ComboBox that changed.
 */
void MTCGenAudioProcessorEditor::comboBoxChanged(juce::ComboBox* cb)
{
    if (cb == &frameRateComboBox)
    {
        int id = frameRateComboBox.getSelectedId();
        processor.setFrameRate(id == 1 ? 24.0 : id == 2 ? 25.0 : id == 3 ? 29.97 : 30.0);
    }
    else if (cb == &mtcFormatComboBox)
    {
        processor.setMTCFormat(
            mtcFormatComboBox.getSelectedId() == 1 ? FullSysEx : QuarterFrame);
    }
}

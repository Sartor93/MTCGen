/*
  ==============================================================================

    MappingEntry.h
    Created: 15 Apr 2025
    Author:  Sebastian

  ==============================================================================
*/

#ifndef MAPPINGENTRY_H_INCLUDED
#define MAPPINGENTRY_H_INCLUDED

#include <JuceHeader.h>

/**
 * @brief Represents a mapping between a MIDI note and a base (preset) timecode,
 * along with the detected note–on and note–off times.
 */
class MappingEntry
{
public:
    MappingEntry(const juce::String& timecode = "00:10:00:00",
        int midiNote = 60,
        const juce::String& labelText = "")
        : timecodeString(timecode),
        midiNote(midiNote),
        label(labelText),
        detectedStartTime(-1.0),
        detectedEndTime(-1.0),
        isActive(false)
    {
    }

    ~MappingEntry() {}

    // Editable fields:
    const juce::String& getTimecodeString() const { return timecodeString; }
    void setTimecodeString(const juce::String& newTimecode) { timecodeString = newTimecode; }

    int getMidiNote() const { return midiNote; }
    void setMidiNote(int newNote) { midiNote = newNote; }

    const juce::String& getLabel() const { return label; }
    void setLabel(const juce::String& newLabel) { label = newLabel; }

    // Detected start and end times:
    double getDetectedStartTime() const { return detectedStartTime; }
    void setDetectedStartTime(double t) { detectedStartTime = t; }

    double getDetectedEndTime() const { return detectedEndTime; }
    void setDetectedEndTime(double t) { detectedEndTime = t; }

    bool getIsActive() const { return isActive; }
    void setIsActive(bool b) { isActive = b; }

    /**
     * @brief Converts the mapping’s preset timecode (HH:MM:SS:FF) into seconds,
     * using the given frame rate.
     */
    double getTimeInSeconds(double frameRate) const;

    // XML serialization
    juce::XmlElement* createXml() const;
    void loadFromXml(const juce::XmlElement& xml);

private:
    juce::String timecodeString;
    int midiNote;
    juce::String label;

    double detectedStartTime;
    double detectedEndTime;
    bool isActive;
};

#endif // MAPPINGENTRY_H_INCLUDED

/*
  ==============================================================================

    MappingEntry.cpp
    Created: 15 Apr 2025
    Author:  Sebastian

  ==============================================================================
*/

#include "MappingEntry.h"

double MappingEntry::getTimeInSeconds(double frameRate) const
{
    auto parts = juce::StringArray::fromTokens(timecodeString, ":", "");
    if (parts.size() != 4)
        return 0.0;

    int hours = parts[0].getIntValue();
    int minutes = parts[1].getIntValue();
    int seconds = parts[2].getIntValue();
    int frames = parts[3].getIntValue();
    return hours * 3600 + minutes * 60 + seconds + frames / frameRate;
}

juce::XmlElement* MappingEntry::createXml() const
{
    auto* xml = new juce::XmlElement("MappingEntry");
    xml->setAttribute("timecode", timecodeString);
    xml->setAttribute("midiNote", midiNote);
    xml->setAttribute("label", label);
    xml->setAttribute("detectedStartTime", detectedStartTime);
    xml->setAttribute("detectedEndTime", detectedEndTime);
    // isActive is transient and not saved.
    return xml;
}

void MappingEntry::loadFromXml(const juce::XmlElement& xml)
{
    if (xml.hasAttribute("timecode"))
        timecodeString = xml.getStringAttribute("timecode");
    if (xml.hasAttribute("midiNote"))
        midiNote = xml.getIntAttribute("midiNote");
    if (xml.hasAttribute("label"))
        label = xml.getStringAttribute("label");
    detectedStartTime = xml.getDoubleAttribute("detectedStartTime", -1.0);
    detectedEndTime = xml.getDoubleAttribute("detectedEndTime", -1.0);
}

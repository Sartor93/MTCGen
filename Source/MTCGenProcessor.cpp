/**
 * @file MTCGenProcessor.cpp
 * @brief Definitions for MTCGenAudioProcessor methods.
 */

#include "MTCGenProcessor.h"
#include "MTCGenEditor.h"
#include <deque>

 //==============================================================================
MTCGenAudioProcessor::MTCGenAudioProcessor()
    : AudioProcessor(BusesProperties())
{
    mappings.emplace_back("00:10:00:00", 60, "Default Mapping");
}

MTCGenAudioProcessor::~MTCGenAudioProcessor()
{
    stopTimer();
}

//==============================================================================
// Boilerplate: state, programs, etc.
const juce::String MTCGenAudioProcessor::getName() const { return "MTCGen"; }
bool MTCGenAudioProcessor::acceptsMidi() const { return true; }
bool MTCGenAudioProcessor::producesMidi() const { return true; }
bool MTCGenAudioProcessor::isMidiEffect() const { return true; }
double MTCGenAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int MTCGenAudioProcessor::getNumPrograms() { return 1; }
int MTCGenAudioProcessor::getCurrentProgram() { return 0; }
void MTCGenAudioProcessor::setCurrentProgram(int) {}
const juce::String MTCGenAudioProcessor::getProgramName(int) { return {}; }
void MTCGenAudioProcessor::changeProgramName(int, const juce::String&) {}

void MTCGenAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto xml = std::make_unique<juce::XmlElement>("MTCGenState");
    for (auto& m : mappings)
        xml->addChildElement(m.createXml());
    xml->setAttribute("frameRate", frameRate);
    xml->setAttribute("mtcFormat", (int)mtcFormat);
    copyXmlToBinary(*xml, destData);
}

void MTCGenAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xmlState = getXmlFromBinary(data, sizeInBytes))
    {
        mappings.clear();
        forEachXmlChildElement(*xmlState, e)
        {
            MappingEntry m;
            m.loadFromXml(*e);
            mappings.push_back(m);
        }
        frameRate = xmlState->getDoubleAttribute("frameRate", frameRate);
        setMTCFormat((MTCFormat)xmlState->getIntAttribute("mtcFormat", (int)mtcFormat));
    }
}

bool MTCGenAudioProcessor::isBusesLayoutSupported(const BusesLayout&) const { return true; }

void MTCGenAudioProcessor::prepareToPlay(double sampleRate, int)
{
    currentSampleRate = sampleRate;
    internalTime = 0.0;
    if (mtcFormat == QuarterFrame)
    {
        auto intervalMs = int(1000.0 / (frameRate * 8.0));
        startTimer(intervalMs);
    }
}

void MTCGenAudioProcessor::releaseResources()
{
    stopTimer();
}

//==============================================================================
/**
 * @brief Queries the host playhead; on failure, returns last internalTime.
 * @return Time in seconds.
 */
double MTCGenAudioProcessor::getPlayheadTime() const
{
    juce::AudioPlayHead::CurrentPositionInfo pos;
    if (auto* ph = getPlayHead(); ph && ph->getCurrentPosition(pos))
        return pos.timeInSeconds;
    return internalTime;
}

//==============================================================================
/**
 * @brief Audio/MIDI callback: logs NoteOn/Off, arms/disarms mappings, and emits MTC.
 */
void MTCGenAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // 1) Update internalTime from host playhead
    juce::AudioPlayHead::CurrentPositionInfo pos;
    if (auto* ph = getPlayHead(); ph && ph->getCurrentPosition(pos))
        internalTime = pos.timeInSeconds;
    else
        internalTime += buffer.getNumSamples() / currentSampleRate;

    // 2) Handle incoming MIDI
    for (auto meta : midiMessages)
    {
        auto msg = meta.getMessage();
        double tstamp = internalTime + meta.samplePosition / currentSampleRate;

        if (msg.isNoteOn())
        {
            int note = msg.getNoteNumber();
            startMappingForNote(note, tstamp);
            addDebugEvent("NoteOn  " + juce::MidiMessage::getMidiNoteName(note, true, true, 4),
                tstamp);
        }
        else if (msg.isNoteOff())
        {
            int note = msg.getNoteNumber();
            stopMappingForNote(note);
            addDebugEvent("NoteOff " + juce::MidiMessage::getMidiNoteName(note, true, true, 4),
                tstamp);
        }
    }

    // 3) Generate Full SysEx MTC if active
    if (mtcFormat == FullSysEx)
    {
        if (auto* m = findActiveMapping(internalTime))
        {
            double elapsed = internalTime - m->getDetectedStartTime();
            double baseSecs = m->getTimeInSeconds(frameRate);
            double outTime = baseSecs + elapsed;

            int totalSecs = int(outTime);
            int hh = totalSecs / 3600;
            int mm = (totalSecs % 3600) / 60;
            int ss = totalSecs % 60;
            int ff = int((outTime - totalSecs) * frameRate);

            uint8_t sx[10] = {
                0xF0,0x7F,0x7F,0x01,0x01,
                (uint8_t)hh,(uint8_t)mm,
                (uint8_t)ss,(uint8_t)ff,0xF7
            };
            auto sysExMsg = juce::MidiMessage::createSysExMessage(sx, 10);
            midiMessages.addEvent(sysExMsg, 0);
            for (auto* out : midiOutputs)
                if (out) out->sendMessageNow(sysExMsg);
        }
    }
}

//==============================================================================
/**
 * @brief Locates which mapping to drive based on live Note-On or stored window.
 * @param hostTime Current host time in seconds.
 * @return Pointer to MappingEntry or nullptr.
 */
MappingEntry* MTCGenAudioProcessor::findActiveMapping(double hostTime)
{
    // 1) If a Note-On is live, keep driving that mapping (even if hostTime ≤ start).
    for (auto& m : mappings)
        if (m.getIsActive())
            return &m;

    // 2) Otherwise only auto-start if we've jumped _into_ the stored [start,end] window.
    //    Note the strict > and <, so hostTime must be strictly inside.
    for (int i = 0; i < (int)mappings.size(); ++i)
    {
        auto& m = mappings[i];
        double start = m.getDetectedStartTime();
        double end = m.getDetectedEndTime(); // –1 if still held

        if (start < 0.0)                continue;  // never set
        if (hostTime <= start)         continue;  // started at or before the note-on
        if (end >= 0.0 && hostTime >= end) continue;  // past the note-off

        activeMappingIndex = i;
        return &m;
    }

    return nullptr;
}


//==============================================================================
/**
 * Called on the UI‐thread timer to update/clear mappings when playback stops or scrubs,
 * and to compute the currentTimecode for any active mapping.
 */
void MTCGenAudioProcessor::updateTimecodeFromPlayHead()
{
    double hostTime = getPlayheadTime();

    // 1) Detect a backward jump (stop or scrub) and record implicit End-times:
    if (hostTime < lastPlayheadTime)
    {
        for (auto& m : mappings)
        {
            if (m.getIsActive() && m.getDetectedEndTime() < 0.0)
            {
                // Stop at the last position
                m.setDetectedEndTime(lastPlayheadTime);
                m.setIsActive(false);
            }
        }
        activeMappingIndex = -1;
        currentTimecode.clear();
    }

    lastPlayheadTime = hostTime;
    internalTime = hostTime;

    // 2) Find which mapping to drive (either live Note-On, or a window you jumped into):
    juce::ScopedLock sl(timecodeLock);
    MappingEntry* m = findActiveMapping(hostTime);

    if (m != nullptr)
    {
        // compute base + elapsed
        double elapsed = hostTime - m->getDetectedStartTime();
        double baseSeconds = m->getTimeInSeconds(frameRate);
        double outTime = baseSeconds + elapsed;

        int totalSecs = int(outTime);
        int hh = totalSecs / 3600;
        int mm = (totalSecs % 3600) / 60;
        int ss = totalSecs % 60;
        int ff = int((outTime - totalSecs) * frameRate);

        currentTimecode = juce::String::formatted("%02d:%02d:%02d:%02d", hh, mm, ss, ff);
    }
    else
    {
        currentTimecode.clear();
    }
}



//==============================================================================
void MTCGenAudioProcessor::setMTCFormat(MTCFormat fmt)
{
    mtcFormat = fmt;
    quarterFrameIndex = 0;
    if (fmt == QuarterFrame)
        startTimer(int(1000.0 / (frameRate * 8.0)));
    else
        stopTimer();
}

void MTCGenAudioProcessor::hiResTimerCallback() { sendQuarterFrame(); }

/** @brief Unchanged quarter-frame handling. */
void MTCGenAudioProcessor::sendQuarterFrame() { /* ... */ }

void MTCGenAudioProcessor::startMappingForNote(int midiNote, double startTime)
{
    for (auto& m : mappings)
        if (m.getMidiNote() == midiNote)
        {
            m.setDetectedStartTime(startTime);
            m.setDetectedEndTime(-1.0);
            m.setIsActive(true);
            break;
        }
}

void MTCGenAudioProcessor::stopMappingForNote(int midiNote)
{
    for (auto& m : mappings)
        if (m.getMidiNote() == midiNote)
        {
            m.setDetectedEndTime(internalTime);
            m.setIsActive(false);
            break;
        }
}

juce::String MTCGenAudioProcessor::getCurrentTimecode()
{
    const juce::ScopedLock sl(timecodeLock);
    return currentTimecode;
}

juce::StringArray MTCGenAudioProcessor::getAvailableMidiOutputNames() const
{
    juce::StringArray list;
    for (auto& d : juce::MidiOutput::getAvailableDevices())
        list.add(d.name);
    return list;
}

void MTCGenAudioProcessor::removeMapping(int index)
{
    if (index >= 0 && index < (int)mappings.size())
        mappings.erase(mappings.begin() + index);
}

void MTCGenAudioProcessor::setSelectedMidiOutputs(const juce::Array<int>& indices)
{
    selectedMidiOutputIndices = indices;
    midiOutputs.clear();
    auto devices = juce::MidiOutput::getAvailableDevices();
    for (auto idx : indices)
        if (idx >= 0 && idx < (int)devices.size())
            if (auto out = juce::MidiOutput::openDevice(devices[idx].identifier))
                midiOutputs.add(out.release());
}

void MTCGenAudioProcessor::addDebugEvent(const juce::String& desc, double time)
{
    debugEvents.push_back({ desc, time });
    if (debugEvents.size() > 5)
        debugEvents.pop_front();
}

std::vector<MTCGenAudioProcessor::MidiEventInfo> MTCGenAudioProcessor::getDebugEvents() const
{
    return { debugEvents.begin(), debugEvents.end() };
}

//==============================================================================
// Change frame rate and restart/stop the high-res timer if in QuarterFrame mode
void MTCGenAudioProcessor::setFrameRate(double newRate)
{
    frameRate = newRate;
    quarterFrameIndex = 0;

    if (mtcFormat == QuarterFrame)
        startTimer(int(1000.0 / (frameRate * 8.0)));
    else
        stopTimer();
}


juce::AudioProcessorEditor* MTCGenAudioProcessor::createEditor() { return new MTCGenAudioProcessorEditor(*this); }
bool MTCGenAudioProcessor::hasEditor() const { return true; }

/**
 * @brief JUCE plugin factory function.
 *
 * This is the entry point JUCE (and the VST3 wrapper) looks for
 * when loading the plugin DLL. It must return a new instance of
 * your AudioProcessor subclass.
 *
 * @return A heap‐allocated MTCGenAudioProcessor.
 */
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MTCGenAudioProcessor();
}
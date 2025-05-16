/**
 * @file MTCGenProcessor.h
 * @brief Declaration of the MTCGenAudioProcessor class that generates MIDI Timecode (MTC).
 */

#ifndef MTCGENPROCESSOR_H_INCLUDED
#define MTCGENPROCESSOR_H_INCLUDED

#include <JuceHeader.h>
#include <vector>
#include <deque>
#include "MappingEntry.h"

 /**
  * @enum MTCFormat
  * @brief The type of MTC output to use.
  */
enum MTCFormat
{
    FullSysEx,   /**< 10-byte SysEx messages */
    QuarterFrame /**< Quarter-frame MTC messages */
};

/**
 * @class MTCGenAudioProcessor
 * @brief JUCE AudioProcessor for generating MTC streams based on MIDI note mappings.
 */
class MTCGenAudioProcessor : public juce::AudioProcessor,
    private juce::HighResolutionTimer
{
public:
    /** Constructor */
    MTCGenAudioProcessor();
    /** Destructor */
    ~MTCGenAudioProcessor() override;

    /** @name Standard AudioProcessor Overrides */
    //@{
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override;
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;
    //@}

    /**
     * @brief Accessor for all MIDI-to-timecode mappings.
     * @return Reference to the internal vector of MappingEntry.
     */
    std::vector<MappingEntry>& getMappings() { return mappings; }

    /**
     * @brief Queries the host playhead for current time, falling back to internalTime.
     * @return Time in seconds.
     */
    double getCurrentHostTime() const;

    /**
     * @brief Safely queries the host playhead time (works on UI thread).
     * @return Time in seconds.
     */
    double getPlayheadTime() const;

    /**
     * @brief Called on Note-On: captures startTime and arms this mapping.
     * @param midiNote MIDI note number triggering the mapping.
     * @param startTime Timestamp (s) when note-on occurred.
     */
    void startMappingForNote(int midiNote, double startTime);

    /**
     * @brief Called on Note-Off: captures endTime and disarms this mapping.
     * @param midiNote MIDI note number triggering the mapping.
     */
    void stopMappingForNote(int midiNote);

    /**
     * @brief Returns the most recently computed timecode string.
     * @return "HH:MM:SS:FF" or empty if inactive.
     */
    juce::String getCurrentTimecode();

    /**
     * @brief Removes the mapping at the given index.
     * @param index Index in the mappings vector.
     */
    void removeMapping(int index);

    /**
     * @brief Called periodically by the editor’s timer to update internal timecode.
     */
    void updateTimecodeFromPlayHead();

    /**
     * @brief Sets the timecode frame rate (e.g., 24, 25, 29.97, 30).
     * @param newRate Frames per second.
     */
    void setFrameRate(double newRate);

    /**
     * @brief Retrieves the configured timecode frame rate.
     * @return Frames per second.
     */
    double getFrameRate() const { return frameRate; }

    /**
     * @brief Switches between Full SysEx and Quarter-Frame output.
     * @param fmt Desired MTCFormat.
     */
    void setMTCFormat(MTCFormat fmt);

    /**
     * @brief Retrieves the current MTC format.
     * @return Current MTCFormat.
     */
    MTCFormat getMTCFormat() const { return mtcFormat; }

    /**
     * @brief Index of the mapping currently driving timecode.
     * @return Mapping index or -1 if none.
     */
    int getActiveMappingIndex() const { return activeMappingIndex; }

    /**
     * @brief Lists available system MIDI outputs.
     * @return StringArray of output device names.
     */
    juce::StringArray getAvailableMidiOutputNames() const;

    /**
     * @brief Selects which MIDI outputs receive MTC.
     * @param indices Array of device indices.
     */
    void setSelectedMidiOutputs(const juce::Array<int>&);

    /**
     * @struct MidiEventInfo
     * @brief Holds a debug descriptor and timestamp for recent MIDI events.
     */
    struct MidiEventInfo
    {
        juce::String desc; /**< Human-readable description */
        double       time; /**< Timestamp in seconds */
    };

    /**
     * @brief Fetches the last few MIDI debug events (up to 5).
     * @return Vector of MidiEventInfo.
     */
    std::vector<MidiEventInfo> getDebugEvents() const;

private:
    /** HighResolutionTimer callback (for quarter-frame timing) */
    void hiResTimerCallback() override;

    /** Remembers the last hostTime to detect transport jumps */
    double lastPlayheadTime{ 0.0 };  

    /**
     * @brief Determines which MappingEntry should be active at hostTime.
     * @param hostTime Current playhead time (s).
     * @return Pointer to the chosen MappingEntry or nullptr.
     */
    MappingEntry* findActiveMapping(double hostTime);

    /** Sends the next quarter-frame MTC message. */
    void sendQuarterFrame();

    /**
     * @brief Logs a debug event (NoteOn/Off) with timestamp.
     * @param description Text describing the event.
     * @param time Timestamp in seconds.
     */
    void addDebugEvent(const juce::String& description, double time);

    double currentSampleRate{ 44100.0 };   /**< Audio sample rate (Hz) */
    double frameRate{ 30.0 };             /**< MTC frames per second */
    double internalTime{ 0.0 };           /**< Fallback time source */
    juce::CriticalSection timecodeLock; /**< Protects currentTimecode */
    juce::String currentTimecode;       /**< Last computed "HH:MM:SS:FF" */

    std::vector<MappingEntry> mappings; /**< All user mappings */
    int activeMappingIndex{ -1 };         /**< Currently active mapping */

    juce::Array<int> selectedMidiOutputIndices; /**< Chosen MIDI outputs */
    juce::OwnedArray<juce::MidiOutput> midiOutputs; /**< Open MIDIOutput instances */

    MTCFormat mtcFormat{ FullSysEx };    /**< FullSysEx or QuarterFrame */
    int quarterFrameIndex{ 0 };          /**< Index for quarter-frame sequence */

    std::deque<MidiEventInfo> debugEvents; /**< Rolling log of last 5 events */

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MTCGenAudioProcessor)
};

#endif // MTCGENPROCESSOR_H_INCLUDED

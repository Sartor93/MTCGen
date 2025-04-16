MTCGen

MTCGen is a MIDI‑effect VST3 plugin built with JUCE that lets you map individual MIDI notes to preset start timecodes, then generates synchronized MIDI Timecode (MTC) streams—either Full SysEx or Quarter‑Frame—when those notes play. Perfect for driving light‑show consoles, external recorders, or any device that accepts MIDI Timecode.
Features

    MIDI‑Note→Timecode Mapping
    Define any number of mappings between MIDI notes and base timecodes (HH:MM:SS:FF).

    Manual “Set Start” Detection
    Capture the exact host time for each note‑on with one click.

    Multi‑Format Output
    Choose between standard Full SysEx MTC or high‑resolution Quarter‑Frame messages.

    Selectable MIDI Outputs
    Send your Timecode stream to one or more physical or virtual MIDI ports.

    Adjustable Frame Rate
    Support for 24, 25, 29.97, and 30 fps.

    Persistent State
    Mappings, labels, frame rate, and selected format/ports are saved and restored via XML.

Requirements

    JUCE 7.x (tested with 7.1+)

    C++17‑compatible compiler

    Visual Studio 2022 (Win) or Xcode 12+ (macOS)

    Projucer (to open the project and export your IDE solution)

Building

    Clone this repository.

    Open MTCGen.jucer in the Projucer.

    Under Plugin Characteristics, ensure “Plugin is a MIDI effect” is checked and disable any audio buses.

    Save and Export your IDE project.

    In your IDE, build the Debug and/or Release target.

    Copy the resulting MTCGen.vst3 (or equivalent) into your DAW’s plugin folder.

Usage

    Load MTCGen as a MIDI‑effect in your DAW.

    Add mappings using the Add Mapping button:

        Enter a Label, choose a MIDI Note from the dropdown, and set the Mapping Timecode.

    Place the corresponding MIDI note in your DAW timeline (or route a controller keyboard).

    Select your desired Frame Rate and MTC Format (Full SysEx vs. Quarter‑Frame).

    In MIDI Outputs, check the ports you want to drive.

    When you trigger a mapped note‑on, the plugin will start streaming MTC from the preset timecode until the note‑off.

Use a MIDI loopback (e.g. LoopBe1) and a monitor (e.g. TimeCode Monitor) or your lighting console (ChamSys, ETC, etc.) to verify sync.
Contributing

    Fork the repo & create a feature branch.

    Make your changes, respecting the existing style (JUCE conventions, tabs/spaces).

    Submit a pull request with a clear description of your additions.

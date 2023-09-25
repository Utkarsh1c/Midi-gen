// midi.hpp

#pragma once

#include "MidiFile.h"

#include <string>

#include <vector>

#include <map>

#include <cstdint>

#include <stdexcept>

using namespace smf;

class MIDIHandler {
  public:
    // Constructor to init maps
    MIDIHandler();

  // Destructor
  ~MIDIHandler();

  // Read a MIDI file
  std::vector < std::vector < std::string > > read_midi_file(const std::string & file_name);

  // Write a vector of MidiEvents to a MIDI file
  void write_midi_file(const std::string & file_name,
    const std::vector < MidiEvent > & events);

  // Convert a note name to a MIDI byte ("C4" -> 60)
  uint8_t note_to_byte(const std::string & note);

  // Convert a MIDI byte to a note name (60 -> "C4")
  std::string byte_to_note(uint8_t byte);

  // Convert a chord name to a vector of MIDI bytes ("Cmaj7" -> {60, 64, 67, 71})
  std::vector < uint8_t > chord_to_bytes(const std::string & chord);

  // Convert a vector of MIDI bytes to a chord name ({60, 64, 67, 71} -> "Cmaj7")
  std::string bytes_to_chord(const std::vector < uint8_t > & bytes);

  private:
    // MIDI file object
    MidiFile midifile;

  // Map of note names to MIDI bytes
  std::map < std::string,
  uint8_t > note_map;

  // Map of chord names to vectors of MIDI bytes
  std::map < std::string,
  std::vector < uint8_t >> chord_map;

  // Generate a vector of MIDI bytes for a chord
  std::vector < uint8_t > generate_chord_bytes(const std::string & root,
    const std::string & type);

  // Initialize the note and chord maps
  void init_maps();
};
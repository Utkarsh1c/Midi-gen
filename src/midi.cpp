// midi.cpp

#include "midi.hpp"

// Constructor to init maps
MIDIHandler::MIDIHandler() {
  init_maps();
}

// Destructor
MIDIHandler::~MIDIHandler() {}

// Read a MIDI file
std::vector < std::vector < std::string > > MIDIHandler::read_midi_file(const std::string & file_name) {
  std::vector < MidiEvent > events;
  midifile.read(file_name);
  if (!midifile.status()) {
    throw std::runtime_error("Could not read MIDI file: " + file_name);
  }
  midifile.doTimeAnalysis();
  // Vector of strings to hold the notes or chords
  std::vector < std::vector < std::string > > notes_or_chords;
  // Previous event's tick
  int prev_tick = -1;
  // Current note or chord
  std::vector < std::string > current;

  int tpq = midifile.getTicksPerQuarterNote();

  // Loop through each event in the vector of events

  for (int track = 0; track < midifile.getTrackCount(); track++) {
    for (int event = 0; event < midifile[track].size(); event++) {
      MidiEvent mev = midifile[track][event];

      // Get the tick of the event
      int tick = mev.tick;

      // Check if the event is a note on channel event
      if (mev.isNoteOn()) {
        // Convert the byte of the event to a note name using the midi handler object
        std::string note = byte_to_note(mev[1]);

        // Time difference between the current event and the previous event
        int delta_time = abs(tick - prev_tick);

        // Threshold based on the tempo of the MIDI file
        int threshold = tpq * 0.01;

        // Check if previous event's tick is equal or close enough to the current event's tick
        if (prev_tick == -1 || delta_time < threshold) {
          // Concatenate the note name to the current note or chord
          current.push_back(note);
        } else {
          // Add the current note or chord to the vector
          notes_or_chords.push_back(current);
          // Start a new note or chord with the current note name
          current.clear();
          current.push_back(note);
        }

        // Replace previous event's tick with the current event's tick
        prev_tick = tick;
      }

    }
    notes_or_chords.push_back(current);

  }
  return notes_or_chords;
}

// Write a vector of MidiEvents to a MIDI file
void MIDIHandler::write_midi_file(const std::string & file_name,
  const std::vector < MidiEvent > & events) {
  MidiFile genmidi;
  genmidi.setTicksPerQuarterNote(400);
  // Add a new track for each event
  for (const auto & event: events) {
    genmidi.addTrack(1);
    MidiEvent e = event;
    genmidi[genmidi.getTrackCount() - 1].push_back(e);
  }
  genmidi.write(file_name);
}

// Convert a note name to a MIDI byte ("C4" -> 60)
uint8_t MIDIHandler::note_to_byte(const std::string & note) {
  if (note.empty()) {
    throw std::invalid_argument("Invalid note name: " + note);
  }

  try {
    return note_map.at(note);

  } catch (const std::out_of_range & e) {
    throw std::invalid_argument("Invalid note name: " + note);
  }

}

// Convert a MIDI byte to a note name (60 -> "C4")
std::string MIDIHandler::byte_to_note(uint8_t byte) {
  for (const auto & entry: note_map) {
    if (byte == entry.second) {
      return entry.first;
    }
  }

  throw std::invalid_argument("Invalid MIDI byte: " + std::to_string(byte));
}

// Convert a chord name to a vector of MIDI bytes ("Cmaj7" -> {60, 64, 67, 71})
std::vector < uint8_t > MIDIHandler::chord_to_bytes(const std::string & chord) {
  if (chord.empty()) {
    throw std::invalid_argument("Invalid chord name: " + chord);
  }

  try {
    return chord_map[chord];
  } catch (const std::out_of_range & e) {
    throw std::invalid_argument("Invalid chord name: " + chord);
  }
}

// Convert a vector of MIDI bytes to a chord name ({60, 64, 67, 71} -> "Cmaj7")
std::string MIDIHandler::bytes_to_chord(const std::vector < uint8_t > & bytes) {
  if (bytes.empty()) {
    throw std::invalid_argument("Invalid vector of MIDI bytes");
  }

  if (bytes.size() == 1) {
    return byte_to_note(bytes[0]);
  }

  try {

    const std::vector < uint8_t > & b = bytes;

    std::string s(b.begin(), b.end());
    auto it = chord_map.find(s);

    // Return the chord name if the iterator is valid
    if (it != chord_map.end()) {
      return it -> first;
    } else {
      throw std::invalid_argument("Invalid vector of MIDI bytes");
    }
  } catch (const std::out_of_range & e) {
    throw std::invalid_argument("Invalid vector of MIDI bytes");
  }

}

// Generate a vector of MIDI bytes for a chord
std::vector < uint8_t > MIDIHandler::generate_chord_bytes(const std::string & root,
  const std::string & type) {
  // Root note byte
  uint8_t root_byte = note_to_byte(root);

  // Vector to store the chord bytes
  std::vector < uint8_t > chord_bytes;

  // Adding the root note byte to the vector
  chord_bytes.push_back(root_byte);

  // Adding the corresponding intervals for chord type

  // major chord
  if (type == "maj") {
    chord_bytes.push_back(root_byte + 4);
    chord_bytes.push_back(root_byte + 7);
  }
  // minor chord
  else if (type == "min") {

    chord_bytes.push_back(root_byte + 3);
    chord_bytes.push_back(root_byte + 7);
  } // diminished chord
  else if (type == "dim") {

    chord_bytes.push_back(root_byte + 3);
    chord_bytes.push_back(root_byte + 6);
  }
  // augmented chord
  else if (type == "aug") {

    chord_bytes.push_back(root_byte + 4);
    chord_bytes.push_back(root_byte + 8);
  }
  // suspended second chord
  else if (type == "sus2") {

    chord_bytes.push_back(root_byte + 2);
    chord_bytes.push_back(root_byte + 7);
  }
  // suspended fourth chord
  else if (type == "sus4") {

    chord_bytes.push_back(root_byte + 5);
    chord_bytes.push_back(root_byte + 7);
  }
  // major seventh chord
  else if (type == "maj7") {

    chord_bytes.push_back(root_byte + 4);
    chord_bytes.push_back(root_byte + 7);
    chord_bytes.push_back(root_byte + 11);
  }
  // minor seventh chord
  else if (type == "min7") {

    chord_bytes.push_back(root_byte + 3);
    chord_bytes.push_back(root_byte + 7);
    chord_bytes.push_back(root_byte + 10);
  }
  // dominant seventh chord
  else if (type == "dom7") {

    chord_bytes.push_back(root_byte + 4);
    chord_bytes.push_back(root_byte + 7);
    chord_bytes.push_back(root_byte + 10);
  }
  // diminished seventh chord
  else if (type == "dim7") {

    chord_bytes.push_back(root_byte + 3);
    chord_bytes.push_back(root_byte + 6);
    chord_bytes.push_back(root_byte + 9);

  }
  // half-diminished seventh chord
  else if (type == "halfdim7") {

    chord_bytes.push_back(root_byte + 3);
    chord_bytes.push_back(root_byte + 6);
    chord_bytes.push_back(root_byte + 10);

  }
  // minor major seventh chord
  else if (type == "minmaj7") {

    chord_bytes.push_back(root_byte + 3);
    chord_bytes.push_back(root_byte + 7);
    chord_bytes.push_back(root_byte + 11);

  }
  // augmented major seventh chord
  else if (type == "augmaj7") {

    chord_bytes.push_back(root_byte + 4);
    chord_bytes.push_back(root_byte + 8);
    chord_bytes.push_back(root_byte + 11);
  }
  // augmented seventh chord
  else if (type == "aug7") {

    chord_bytes.push_back(root_byte + 4);
    chord_bytes.push_back(root_byte + 8);
    chord_bytes.push_back(root_byte + 10);

  }
  // Throw error if invalid chord type
  else {
    throw std::invalid_argument("Invalid chord type: " + type);
  }

  // Return the vector of chord bytes
  return chord_bytes;

}

// Initialize the note and chord maps
void MIDIHandler::init_maps() {

  // Initializing the note map with possible note names and MIDI bytes
  note_map = {
    {"C0", 12},
    {"C#0", 13},
    {"Db0", 13},
    {"D0", 14},
    {"D#0", 15},
    {"Eb0", 15},
    {"E0", 16},
    {"F0", 17},
    {"F#0", 18},
    {"Gb0", 18},
    {"G0", 19},
    {"G#0", 20},
    {"Ab0", 20},
    {"A0", 21},
    {"A#0", 22},
    {"Bb0", 22},
    {"B0", 23}
  };

  // Loop till octaves up to 9
  for (int i = 1; i <= 9; i++) {
    for (const auto & entry: note_map) {
      // Get the note name and byte from the iterator
      const std::string & note_name = entry.first;
      uint8_t note_byte = entry.second;

      // Check if the note name ends with zero
      if (note_name.back() == '0') {
        // Replace the zero with the current octave number
        std::string new_note_name = note_name;
        new_note_name.back() = '0' + i;

        // Add 12 * octave number to the new note
        uint8_t new_note_byte = note_byte + i * 12;

        // Insert the new pair into the note map
        note_map[new_note_name] = new_note_byte;
      }
    }
  }

  // Initializing the chord map with possible chord names and MIDI bytes
  std::vector < std::string > roots {
    "C",
    "C#",
    "Db",
    "D",
    "D#",
    "Eb",
    "E",
    "F",
    "F#",
    "Gb",
    "G",
    "G#",
    "Ab",
    "A",
    "A#",
    "Bb",
    "B"
  };

  std::vector < std::string > types {
    "maj",
    "min",
    "dim",
    "aug",
    "sus2",
    "sus4",
    "maj7",
    "min7",
    "dom7",
    "dim7",
    "halfdim7",
    "minmaj7",
    "augmaj7",
    "aug7"
  };

  for (int i = 0; i <= 8; i++) {
    for (const auto & root: roots) {
      for (const auto & type: types) {
        // chord name = root + octave number + type
        std::string chord_name = root + std::to_string(i) + type;

        std::vector < uint8_t > chord_bytes = generate_chord_bytes(root + std::to_string(i), type);

        // Insert the new pair into the chord map
        chord_map[chord_name] = chord_bytes;
      }
    }
  }
}
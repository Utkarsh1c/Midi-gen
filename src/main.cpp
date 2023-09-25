// main.cpp 

#include "midi.hpp"

#include "markov.hpp"

#include <iostream>

#include <sstream>

int main() {

  // Enter the name of the input MIDI files.
  std::vector < std::string > input_files;
  std::string input_file;

  while (1) {
    std::cout << "Enter the name of an input MIDI file (-1 to stop): ";
    std::getline(std::cin, input_file);

    if (input_file != "-1") {
      input_files.push_back(input_file);
    } else {
      break;
    }
  }

  unsigned int seed = 0;

  // Get random seed value if no file selected
  if (!input_files.size()) {
    seed = rand() % 500;
  }

  // MIDI handler object
  MIDIHandler midi_handler;

  // Markov handler object with the random seed value and a reference to the MIDI handler object.
  MarkovHandler markov_handler(midi_handler, seed);

  std::vector < std::string > init_state;

  // Training the Markov model
  for (auto & input_file: input_files) {
    markov_handler.train(input_file);
  }

  // Enter a starting state
  std::cout << "\n \nEnter a starting state as a note or collection of notes separated by a space: ";

  std::string input;
  std::getline(std::cin, input);
  std::stringstream ss(input);

  std::string token;
  std::vector < std::string > start_state;
  while (ss >> token) {
    start_state.push_back(token);
  }

  // Enter the number of notes or chords to generate.
  std::cout << "\n \nEnter number of notes or chords to generate: ";

  int num;
  std::cin >> num;

  std::vector < std::vector < std::string > > generated_notes;
  std::vector < MidiEvent > generated_events;
  std::vector < std::string > current_state = start_state;

  for (int i = 0; i < num; i++) {
    if (current_state.empty()) throw;
    generated_notes.push_back(current_state);
    std::vector < std::string > next_state = markov_handler.get_next(current_state);
    current_state = next_state;
  }

  // Write output to 'output.mid'
  markov_handler.write_midi_file(generated_notes, "output.mid");

  return 0;
}
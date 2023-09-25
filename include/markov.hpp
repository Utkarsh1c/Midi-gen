// markov.hpp

#pragma once

#include "midi.hpp"

#include <boost/graph/adjacency_list.hpp>

#include <boost/random/mersenne_twister.hpp>

#include <boost/random/discrete_distribution.hpp>

#include <boost/random/variate_generator.hpp>

#include <boost/random/uniform_int_distribution.hpp>

#include <string>

#include <map>

#include <stdexcept>

class MarkovHandler {
  public:

    // Constructor that takes a reference to the MIDI handler object and a random seed value
    MarkovHandler(MIDIHandler & midi_handler,
      unsigned int seed);

  // Destructor
  ~MarkovHandler();

  // Train the model from Midi file
  void train(const std::string & file_name);

  // Update the markov chain graph with a vector of strings
  void update_graph(const std::vector < std::vector < std::string >> & user_input);

  // Write to a MIDI file
  void write_midi_file(const std::vector < std::vector < std::string > > & notes,
    const std::string & file_name);

  // Get the next note or chord from the current state
  std::vector < std::string > get_next(const std::vector < std::string > & state);

  private:
    // Reference to the MIDIHandler object
    MIDIHandler & midi_handler;

  // Boost random engine object
  boost::random::mt19937 engine;

  // Defining the boost graph for our model
  typedef boost::property < boost::edge_weight_t,
  int > EdgeWeightProperty;
  typedef boost::adjacency_list < boost::vecS,
  boost::vecS,
  boost::directedS,
  std::vector < std::string > ,
  EdgeWeightProperty > Graph;

  Graph graph;

  // State map for our states
  std::map < std::vector < std::string > ,
  boost::graph_traits < Graph > ::vertex_descriptor > state_map;

  // Add a state to the graph if it does not exist already
  void add_state(const std::vector < std::string > & state);

  // Add a transition to the graph if it does not exist already
  void add_transition(const std::vector < std::string > & from,
    const std::vector < std::string > & to, int weight);
};
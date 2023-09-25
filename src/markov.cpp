// markov.cpp

#include "markov.hpp"

// Constructor that takes a reference to the MIDI handler object and a random seed value
MarkovHandler::MarkovHandler(MIDIHandler &midi_handler,
	unsigned int seed): midi_handler(midi_handler)
{
	if (seed)
	{
		// Initializing the random engine with seed
		engine.seed(seed);

		// Generating random states and transitions for model

		std::vector<std::vector<std::string>> states;
		// Number of states 
		int num_states = 100;
		// Range of valid midi bytes in accordance to our maps
		int min_byte = 12;
		int max_byte = 119;
		// Vector of chord types 
		std::vector<std::string > chord_types = {
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

		// Uniform distribution for selecting random bytes 
		boost::random::uniform_int_distribution < > byte_dist(min_byte, max_byte);	// Uniform distribution for selecting random chord types
		boost::random::uniform_int_distribution < > type_dist(0, chord_types.size() - 1);	// Generator for generating random bytes 
		boost::random::variate_generator<boost::random::mt19937 &, boost::random::uniform_int_distribution < >> byte_gen(engine, byte_dist);	// Generator for generating random chord types
		boost::random::variate_generator<boost::random::mt19937 &, boost::random::uniform_int_distribution < >> type_gen(engine, type_dist);
		// Loop to generate random states 
		for (int i = 0; i < num_states; i++)
		{
			// Variable to store the current note or chord
			std::vector<std::string > current;
			// Coin flip for a note or a chord
			bool is_note = (byte_gen() % 2 == 0);
			if (is_note)
			{
			 	// Generate a random note
				uint8_t byte = byte_gen();
				std::string note = midi_handler.byte_to_note(byte);
				current.push_back(note);
			}
			else
			{
			 	// Generate a random chord
				uint8_t root = byte_gen();
				std::string root_note = midi_handler.byte_to_note(root);
				std::string chord_type = chord_types[type_gen()];
				std::string chord = root_note + chord_type;
				std::vector<uint8_t> chord_bytes = midi_handler.chord_to_bytes(chord);
				for (auto byte: chord_bytes)
				{
					std::string note = midi_handler.byte_to_note(byte);
					current.push_back(note);
				}
			}

			states.push_back(current);
			current.clear();
		}

		// Add the random states to the graph
		for (auto state: states)
		{
			add_state(state);
		}

		// Rrandom transitions between the states
		std::vector<std::pair<std::vector< std::string >, std::vector< std::string >>> transitions;
		// Number of transitions to generate
		int num_transitions = num_states *num_states / 2;
		// Range of valid weights for transitions 
		int min_weight = 1;
		int max_weight = num_states;
		// Uniform distribution for selecting random states
		boost::random::uniform_int_distribution < > state_dist(0, num_states - 1);
		// Uniform distribution for assigning random weights 
		boost::random::uniform_int_distribution < > weight_dist(min_weight, max_weight);	// Generator for selecting random states 
		boost::random::variate_generator<boost::random::mt19937 &, 			boost::random::uniform_int_distribution < >> state_gen(engine, state_dist);	// Generator for assigning random weights 
		boost::random::variate_generator<boost::random::mt19937 &, boost::random::uniform_int_distribution < >> weight_gen(engine, weight_dist);
		// Loop to generate random transitions 
		for (int i = 0; i < num_transitions; i++)
		{
			// Selecting two random states from the state_map 
			std::vector<std::string > from = states[state_gen()];
			std::vector<std::string > to = states[state_gen()];

			// Add the transition to the vector 
			transitions.push_back(std::make_pair(from, to));
		}

		// Add the random transitions to the graph
		for (auto transition: transitions)
		{
			std::vector<std::string > from = transition.first;
			std::vector<std::string > to = transition.second;
			// Assigning a random weight to the transition 
			int weight = weight_gen();
			add_transition(from, to, weight);
		}

		std::cout << "Generated states : \n";
		for (auto x: states)
		{
			for (auto y: x)
			{
				std::cout << y << " ";
			}
			std::cout << "\t";
		}

		// Updating the graph with the states
		update_graph(states);
	}
}

// Destructor
MarkovHandler::~MarkovHandler() {}

// Train the model from MIDI file
void MarkovHandler::train(const std::string &file_name)
{
	try
	{
		// Vector of strings that will hold the notes or chords from the events
		std::vector<std::vector<std::string>> notes_or_chords = midi_handler.read_midi_file(file_name);

    std::cout << "Generated states : \n";
		for (auto x: notes_or_chords)
		{
			for (auto y: x)
			{
				std::cout << y << " ";
			}
			std::cout << "\t";
		}

		// Updating the graph with the vector of strings
		update_graph(notes_or_chords);
	}

	// Catch any exceptions
	catch (const std::exception &e)
	{
		std::cerr << "Error while initializing markov model from MIDI file: " << e.what() << std::endl;
		exit(1);
	}
}

// Update the markov chain graph with a vector of strings
void MarkovHandler::update_graph(const std::vector<std::vector<std::string>> &user_input)
{
	if (!user_input.empty())
	{
		for (auto it = user_input.begin(); it != user_input.end() - 1; ++it)
		{
			// Get current and next state from the iterator
			std::vector<std::string > current = *it;
			std::vector<std::string > next = *(it + 1);

			// Add the current and next states to the graph
			try
			{
				add_state(current);
				add_state(next);
			}

			// Catch any exceptions that might occur while adding states
			catch (const std::exception &e)
			{
				std::cerr << "Error while adding states: " << e.what() << std::endl;
				exit(1);
			}

			// Add a transition from the current state to the next state in the graph and increment its weight
			try
			{
				add_transition(current, next, 1);
			}

			// Catch any exceptions
			catch (const std::exception &e)
			{
				std::cerr << "Error while adding transitions: " << e.what() << std::endl;
				exit(1);
			}
		}
	}
}

// Write to a MIDI file
void MarkovHandler::write_midi_file(const std::vector<std::vector<std::string>> &notes, const std::string &file_name)
{
	// If the vector of strings is not empty
	if (!notes.empty())
	{
		// Vector of MidiEvents to hold the events corresponding to the notes or chords

		std::vector<MidiEvent> events;
		int time = 0;
		for (const auto &note_or_chord: notes)
		{
			for (const auto &note: note_or_chord)
			{
			 	// Convert the note to a byte
				uint8_t byte = midi_handler.note_to_byte(note);
				// MidiEvent object for note on
				MidiEvent noteOn;
				noteOn.tick = time;
				// Command byte 0x90 for note on
				noteOn.push_back(0x90);

				noteOn.push_back(byte);
				noteOn.push_back(64);
				events.push_back(noteOn);
			}

			// Random time increment between 60 and 180 ticks
			boost::random::uniform_int_distribution < > dist(60, 180);
			boost::random::variate_generator<boost::random::mt19937 &, 				boost::random::uniform_int_distribution < >> time_gen(engine, dist);
			time += time_gen();

			for (const auto &note: note_or_chord)
			{
			 	// Convert the note to a byte
				uint8_t byte = midi_handler.note_to_byte(note);

				// Note off event for same byte
				MidiEvent noteOff;
				noteOff.tick = time;
				noteOff.push_back(0x80);
				// Command byte 0x80 for note off

				noteOff.push_back(byte);
				noteOff.push_back(64);
				events.push_back(noteOff);
			}

			time += 120;
		}

		// Write the vector of events to a MIDI file
		try
		{
			midi_handler.write_midi_file(file_name, events);
		}

		// Catch any exceptions
		catch (const std::exception &e)
		{
			std::cerr << "Error while writing markov model to MIDI file: " << e.what() << std::endl;
			exit(1);
		}
	}
}

// Get the next note or chord from the current state
std::vector<std::string > MarkovHandler::get_next(const std::vector<std::string > &state)
{
	// If the state exists in the graph or not
	if (state.empty() || state_map.find(state) == state_map.end())
	{
		throw std::invalid_argument("Invalid state");
	}

	// Get the vertex from the state map
	auto vertex = state_map[state];

	// Get the out edges of the vertex
	auto out_edges = boost::out_edges(vertex, graph);

	if (out_edges.first == out_edges.second)
	{
		// If the vertex has no out edges, return an empty vector as the next state
		return {};

	}

	// vector of weights for out edges
	std::vector<int> weights;

	for (auto it = out_edges.first; it != out_edges.second; ++it)
	{
		// Get the edge weight from graph
		int weight = boost::get(boost::edge_weight, graph, *it);
		weights.push_back(weight);
	}

	// Discrete distribution for vector of weights
	boost::random::discrete_distribution < > distribution(weights);
	int index = distribution(engine);

	// Target vertex of the out edge from the index and its state
	auto target = boost::target(*(out_edges.first + index), graph);

	// Get the state from the graph
	std::vector<std::string > next = graph[target];

	// Return the next state
	return next;
}

// Add a state to the graph if it does not exist already
void MarkovHandler::add_state(const std::vector<std::string > &state)
{
	if (state_map.find(state) == state_map.end())
	{
		// Add a new vertex to the graph with the state as its property
		boost::graph_traits<Graph>::vertex_descriptor v = boost::add_vertex(state, graph);
		// Add the state and the vertex to the state map
		state_map[state] = v;
	}
}

// Add a transition to the graph if it does not exist already
void MarkovHandler::add_transition(const std::vector<std::string > &from, const std::vector<std::string > &to, int weight)
{
	// Get the vertices from the state map 
	boost::graph_traits<Graph>::vertex_descriptor u = state_map[from];
	boost::graph_traits<Graph>::vertex_descriptor v = state_map[to];

	std::pair<boost::graph_traits<Graph>::edge_descriptor, bool> e = boost::edge(u, v, graph);
	if (e.second)
	{
		// The edge already exists, increment its weight by one
		int weight = boost::get(boost::edge_weight, graph, e.first);
		boost::put(boost::edge_weight, graph, e.first, weight + 1);
	}
	else
	{
		// The edge does not exist, create it with a weight of one
		boost::add_edge(u, v, EdgeWeightProperty(weight), graph);
	}
}
# Midi-gen
A midi music generator using the Markov model in C++

In this project, a user can train a Markov model based on C++ using some random states or by providing midi files as input to generate an output midi file resulting from the randomized graph states of the model.
To handle chords and notes simultaneously, a modified approach to boost graphs has been used so as to handle a vector of notes that can be used to represent both a single note and a chord.
The user just needs the boost library to build and run the project; all the file handling libraries are included in the project itself using the open-source Midifile library.
The entire project runs on a terminal, with the future scope of adding a terminal fretboard GUI using FTXUI.

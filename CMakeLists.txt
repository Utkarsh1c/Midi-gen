# minimum version of CMake required
cmake_minimum_required(VERSION 3.16)

# project name and version
project(midi_gen VERSION 1.0)

# c++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/midi
    )

file(GLOB midi_SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/midi/*.cpp")

# Add your own source files
set(
    SOURCE_FILES src/main.cpp src/midi.cpp src/markov.cpp
    ${midi_SRC}
    )

# Add your own executable or library target
add_executable(midi_gen ${SOURCE_FILES})

# Set the output directory of the executable target
set_target_properties(midi_gen PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)

# Add some compiler flags for your executable target
target_compile_options(midi_gen PRIVATE -Wall -Wextra -pedantic)
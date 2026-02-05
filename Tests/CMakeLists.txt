# Tests CMakeLists.txt
cmake_minimum_required(VERSION 3.22)

# Fetch Catch2
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

# Create test executable
add_executable(OscillatorTests
    OscillatorTests.cpp
    ${CMAKE_SOURCE_DIR}/Source/dsp/Oscillator.cpp
)

# Include directories
target_include_directories(OscillatorTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/Source/core
    ${CMAKE_SOURCE_DIR}/Source/dsp
)

# Link libraries
target_link_libraries(OscillatorTests PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_dsp
)

# Set C++ standard
target_compile_features(OscillatorTests PRIVATE cxx_std_17)

# Create envelope test executable
add_executable(EnvelopeTests
    EnvelopeTests.cpp
    ${CMAKE_SOURCE_DIR}/Source/dsp/Envelope.cpp
)

# Include directories
target_include_directories(EnvelopeTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/Source/core
    ${CMAKE_SOURCE_DIR}/Source/dsp
)

# Link libraries
target_link_libraries(EnvelopeTests PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_dsp
)

# Set C++ standard
target_compile_features(EnvelopeTests PRIVATE cxx_std_17)

# Create ladder filter test executable
add_executable(LadderFilterTests
    LadderFilterTests.cpp
    ${CMAKE_SOURCE_DIR}/Source/dsp/LadderFilter.cpp
)

# Include directories
target_include_directories(LadderFilterTests PRIVATE
    ${CMAKE_SOURCE_DIR}/Source
    ${CMAKE_SOURCE_DIR}/Source/core
    ${CMAKE_SOURCE_DIR}/Source/dsp
)

# Link libraries
target_link_libraries(LadderFilterTests PRIVATE
    Catch2::Catch2WithMain
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_dsp
)

# Set C++ standard
target_compile_features(LadderFilterTests PRIVATE cxx_std_17)

# Enable testing
include(CTest)
include(Catch)
catch_discover_tests(OscillatorTests)
catch_discover_tests(EnvelopeTests)
catch_discover_tests(LadderFilterTests)

cmake_minimum_required(VERSION 3.22)

# Project definition
project(MicroAcid303 VERSION 1.0.0)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add JUCE
add_subdirectory(JUCE)

# Platform-specific plugin formats
# AU (Audio Unit) only works on macOS
# VST3 works on both Windows and macOS
if(APPLE)
    set(PLUGIN_FORMATS VST3 AU Standalone)
elseif(WIN32)
    set(PLUGIN_FORMATS VST3 Standalone)
else()
    set(PLUGIN_FORMATS VST3 Standalone)
endif()

# Plugin definition
juce_add_plugin(MicroAcid303
    COMPANY_NAME "AcidAudio"
    BUNDLE_ID com.acidaudio.microacid303
    PLUGIN_MANUFACTURER_CODE MAud
    PLUGIN_CODE MA03
    FORMATS ${PLUGIN_FORMATS}
    PRODUCT_NAME "303 Micro Acid"

    # Plugin characteristics
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    # Installation
    COPY_PLUGIN_AFTER_BUILD TRUE

    # VST3 categories
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice
)

# Source files - will add these incrementally
target_sources(MicroAcid303 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/dsp/Oscillator.cpp
    Source/dsp/Envelope.cpp
    Source/dsp/LadderFilter.cpp
)

# Compiler definitions
target_compile_definitions(MicroAcid303 PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
    JUCE_REPORT_APP_USAGE=0
)

# Link JUCE modules
target_link_libraries(MicroAcid303 PRIVATE
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_dsp
    juce::juce_gui_basics
PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

# Platform-specific settings
if(APPLE)
    # macOS specific settings
    set_target_properties(MicroAcid303 PROPERTIES
        XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++17"
        XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++"
    )
elseif(WIN32)
    # Windows-specific compiler definitions
    target_compile_definitions(MicroAcid303 PRIVATE
        _USE_MATH_DEFINES      # Enable M_PI and other math constants
        NOMINMAX               # Prevent Windows.h min/max macros
        WIN32_LEAN_AND_MEAN    # Faster Windows.h compilation
    )

    # MSVC compiler options
    if(MSVC)
        target_compile_options(MicroAcid303 PRIVATE
            /W3           # Warning level 3
            /wd4100       # Unreferenced formal parameter (common in JUCE)
            /MP           # Multi-processor compilation for faster builds
        )
    endif()
endif()

# Enable testing
enable_testing()
add_subdirectory(Tests)

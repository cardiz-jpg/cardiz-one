cmake_minimum_required(VERSION 3.22)
project(CardizOne VERSION 0.6.0 LANGUAGES CXX)

if(APPLE)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version" FORCE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(CardizOne
  COMPANY_NAME "Cardiz On The Beat"
  BUNDLE_ID "com.cardizonthebeat.cardizone"
  PLUGIN_MANUFACTURER_CODE Crdz
  PLUGIN_CODE COne
  FORMATS VST3 AU Standalone
  PRODUCT_NAME "CARDIZ ONE"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE
  IS_MIDI_EFFECT FALSE
  EDITOR_WANTS_KEYBOARD_FOCUS FALSE
  COPY_PLUGIN_AFTER_BUILD FALSE
  VST3_CATEGORIES Fx Mastering)

target_sources(CardizOne PRIVATE
  Source/PluginProcessor.cpp Source/PluginProcessor.h
  Source/PluginEditor.cpp Source/PluginEditor.h)

target_compile_definitions(CardizOne PUBLIC
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(CardizOne PRIVATE
  juce::juce_audio_utils
  juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags
         juce::juce_recommended_lto_flags
         juce::juce_recommended_warning_flags)

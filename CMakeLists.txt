cmake_minimum_required(VERSION 3.1)
project(audioPlugin
    VERSION 1.0
    DESCRIPTION "AudioPlayer plugin Cmake"
    LANGUAGES CXX)
set(CMAKE_BUILD_TYPE Release)
include_directories(.)
set(SOURCES "")
list(APPEND SOURCES
    audioplayer.cpp
    audioPlugin.cpp)

add_library(audioPlugin SHARED ${SOURCES})
target_link_libraries(audioPlugin pthread)
target_link_libraries(audioPlugin pulse-simple)
target_link_libraries(audioPlugin samplerate)
#target_link_libraries(audioPlugin pulse)

set(CMAKE_CXX_FLAGS_RELEASE "-O2")
# Install library
install(TARGETS ${PROJECT_NAME} DESTINATION /opt/vmbase/extensions )

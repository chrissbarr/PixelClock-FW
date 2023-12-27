# PixelClock-FW Desktop Build Main CMake

## [main]

cmake_minimum_required(VERSION 3.2...3.27)

project(
  PixelClock
  VERSION 1.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(PIXELCLOCK_DESKTOP ARDUINO_ARCH_ESP32)

## Dependencies
include(cmake/SFML.cmake)
include(cmake/FMT.cmake)
include(cmake/Button2.cmake)
include(cmake/ETL.cmake)
include(cmake/ArduinoFFT.cmake)

add_subdirectory(lib_desktop/ArduinoStub)

add_subdirectory(lib/FastLEDMath)
target_link_libraries(FastLEDMath ArduinoStub)

target_link_libraries(Button2 ArduinoStub)

set(main_sources
src/display/effects/gravity.cpp
src/display/effects/clockfaces.cpp
src/display/effects/audiowaterfall.cpp
src/display/effects/spectrumdisplay.cpp
src/display/effects/volumedisplay.cpp
src/display/effects/volumegraph.cpp
src/display/effects/bouncingball.cpp
src/display/effects/randomfill.cpp
src/display/effects/filters.cpp
src/display/effects/gravity.cpp
src/display/effects/gravityfill.cpp
src/display/effects/textscroller.cpp
src/display/effects/utilities.cpp
src/display/canvas.cpp
src/loopTimeManager.cpp
src/modes.cpp
src/timekeeping.cpp
src/utility.cpp
src/audio/audio.cpp
src/audio/desktop.cpp
)

add_library(Main STATIC "${main_sources}")
target_include_directories(Main PUBLIC include)
target_link_libraries(Main PUBLIC FastLEDMath)
target_link_libraries(Main PUBLIC fmt::fmt)
target_link_libraries(Main PUBLIC Button2)
target_link_libraries(Main PUBLIC ArduinoFFT)
target_link_libraries(Main PUBLIC etl::etl)
target_link_libraries(Main PUBLIC sfml-graphics sfml-audio)

## Main executable
add_executable(PixelClock src/main.cpp)

# Link libraries
target_link_libraries(PixelClock PRIVATE ArduinoStub)
target_link_libraries(PixelClock PRIVATE Main)

if(WIN32)
    add_custom_command(
        TARGET PixelClock
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:PixelClock>
        VERBATIM)
endif()

install(TARGETS PixelClock)

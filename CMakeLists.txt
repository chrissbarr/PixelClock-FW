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
src/audio/audio.cpp
src/audio/desktop.cpp
src/display/canvas.cpp
src/display/diagnostic.cpp
src/display/effects/audiowaterfall.cpp
src/display/effects/bouncingball.cpp
src/display/effects/clockfaces.cpp
src/display/effects/filters.cpp
src/display/effects/gameoflife.cpp
src/display/effects/gravity.cpp
src/display/effects/gravity.cpp
src/display/effects/gravityfill.cpp
src/display/effects/randomfill.cpp
src/display/effects/spectrumdisplay.cpp
src/display/effects/textscroller.cpp
src/display/effects/utilities.cpp
src/display/effects/volumedisplay.cpp
src/display/effects/volumegraph.cpp
src/instrumentation.cpp
src/loopTimeManager.cpp
src/modes/clockface.cpp
src/modes/effects.cpp
src/modes/modes.cpp
src/modes/settings.cpp
src/timekeeping.cpp
src/utility.cpp
)

add_library(Main STATIC "${main_sources}")
target_include_directories(Main PUBLIC include)
target_link_libraries(Main PUBLIC ArduinoStub)
target_link_libraries(Main PUBLIC FastLEDMath)
target_link_libraries(Main PUBLIC fmt::fmt)
target_link_libraries(Main PUBLIC Button2)
target_link_libraries(Main PUBLIC ArduinoFFT)
target_link_libraries(Main PUBLIC etl::etl)
target_link_libraries(Main PUBLIC sfml-graphics sfml-audio)

if(MSVC)
  target_compile_options(Main PRIVATE /W4 /external:W0)
else()
  target_compile_options(Main PUBLIC -Wall -Wextra -Wpedantic)
endif()

## Main executable
add_executable(PixelClock src/main.cpp)

# Link libraries
target_link_libraries(PixelClock PRIVATE Main)

if(WIN32)
    add_custom_command(
        TARGET PixelClock
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:PixelClock>
        VERBATIM)
endif()

add_custom_command(TARGET PixelClock POST_BUILD
  COMMENT "Copy Data Folder"
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/data $<TARGET_FILE_DIR:PixelClock>/data
)

install(TARGETS PixelClock)

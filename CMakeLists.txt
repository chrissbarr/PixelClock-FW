# CMake simple example

## [main]

# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.2...3.27)

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
  PixelClock
  VERSION 1.0
  LANGUAGES CXX)

# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)
set(CMAKE_CXX_STANDARD 17)
add_compile_definitions(PIXELCLOCK_DESKTOP ARDUINO_ARCH_ESP32)

# Find packages go here.

# You should usually split this into folders, but this is a simple example
include(cmake/SFML.cmake)
include(cmake/FMT.cmake)
include(cmake/Button2.cmake)

add_subdirectory(lib_desktop/ArduinoStub)

add_subdirectory(lib/FastLEDMath)
target_link_libraries(FastLEDMath ArduinoStub)

target_link_libraries(Button2 ArduinoStub)

add_library(Display STATIC lib/display/src/canvas.cpp)
target_include_directories(Display PUBLIC lib/display/include/)
target_link_libraries(Display FastLEDMath)

set(main_sources
src/display/effects/gravity.cpp
src/display/effects/clockfaces.cpp
src/display/effects/bouncingball.cpp
src/display/effects/randomfill.cpp
src/display/effects/filters.cpp
src/display/effects/gravity.cpp
src/display/effects/gravityfill.cpp
src/display/effects/textscroller.cpp
src/display/effects/utilities.cpp
src/loopTimeManager.cpp
src/modes.cpp
src/timekeeping.cpp
src/utility.cpp
)

add_library(Main STATIC "${main_sources}")
target_include_directories(Main PUBLIC include)
target_link_libraries(Main PUBLIC Display)
target_link_libraries(Main PUBLIC fmt::fmt)
target_link_libraries(Main PUBLIC Button2)
# Link each target with other targets or add options, etc.

# Adding something we can run - Output name matches target name
add_executable(MyExample src/main.cpp)

# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error.
target_link_libraries(MyExample PRIVATE ArduinoStub)
target_link_libraries(MyExample PRIVATE Display)
target_link_libraries(MyExample PRIVATE Main)
target_link_libraries(MyExample PRIVATE sfml-graphics)

if(WIN32)
    add_custom_command(
        TARGET MyExample
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:MyExample>
        VERBATIM)
endif()

install(TARGETS MyExample)

## [main]

# This part is so the Modern CMake book can verify this example builds. For your code,
# you'll probably want tests too
enable_testing()
add_test(NAME MyExample COMMAND MyExample)


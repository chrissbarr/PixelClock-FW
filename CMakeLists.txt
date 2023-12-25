# CMake simple example

## [main]

# Almost all CMake files should start with this
# You should always specify a range with the newest
# and oldest tested versions of CMake. This will ensure
# you pick up the best policies.
cmake_minimum_required(VERSION 3.1...3.27)

# This is your project statement. You should always list languages;
# Listing the version is nice here since it sets lots of useful variables
project(
  PixelClock
  VERSION 1.0
  LANGUAGES CXX)

# If you set any CMAKE_ variables, that can go here.
# (But usually don't do this, except maybe for C++ standard)

# Find packages go here.

# You should usually split this into folders, but this is a simple example


add_library(ArduinoStub INTERFACE)
target_include_directories(ArduinoStub INTERFACE lib_desktop/ArduinoStub/include)
#set_target_properties(ArduinoStub PROPERTIES LINKER_LANGUAGE CXX)
#target_compile_definitions(ArduinoStub PUBLIC DESKTOP)

add_library(FastLEDMath lib/FastLEDMath/src/p_hsv2rgb.cpp lib/FastLEDMath/src/p_lib8tion.cpp)
target_include_directories(FastLEDMath PUBLIC lib/FastLEDMath/include/)
target_link_libraries(FastLEDMath ArduinoStub)

add_library(Display lib/display/src/canvas.cpp)
target_include_directories(Display PUBLIC lib/display/include/)
target_link_libraries(Display FastLEDMath)
#target_link_libraries(Display FastLED)


# Link each target with other targets or add options, etc.

# Adding something we can run - Output name matches target name
add_executable(MyExample src/main_desktop.cpp)

# Make sure you link your targets with this command. It can also link libraries and
# even flags, so linking a target that does not exist will not give a configure-time error.
target_link_libraries(MyExample PRIVATE ArduinoStub)
target_link_libraries(MyExample PRIVATE Display)

## [main]

# This part is so the Modern CMake book can verify this example builds. For your code,
# you'll probably want tests too
enable_testing()
add_test(NAME MyExample COMMAND MyExample)


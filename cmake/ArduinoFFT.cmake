include(FetchContent)

FetchContent_Declare(
  ArduinoFFT
  GIT_REPOSITORY https://github.com/HorstBaerbel/arduinoFFT
  GIT_TAG        0a9cd2b4257f4f835d924cafa70b92b23ec5080c
  CONFIGURE_COMMAND
)

FetchContent_Populate(ArduinoFFT)

add_library(ArduinoFFT INTERFACE)
target_include_directories(Button2 INTERFACE "${arduinofft_SOURCE_DIR}/src")